#include "audiohandlers/VorbisManager.hpp"

typedef AudioBufferSPSC<float> FloatBuf;
VorbisManager* VorbisManager::GetVorbisManager(int twopow, char* filename) {
  if (twopow <= 1) {
    // unreasonable
    return nullptr;
  }
  int err;

  if (filename == NULL) {
    return nullptr;
  }

  stb_vorbis* file = stb_vorbis_open_filename(filename, &err, NULL);
  if (err != VORBIS__no_error) {
    // error occurred
    return nullptr;
  }

  // opened valid -- call constructor
  return new VorbisManager(twopow, file);
}

ReadOnlyBuffer* VorbisManager::CreateBufferInstance() {
  // realization: why would we ever want a larger buffer
  // the size of the critical buffer affects the number of terms in all others
  std::shared_ptr<FloatBuf> result(new FloatBuf(buffer_power_, channel_count_));
  std::lock_guard lock(buffer_list_lock_);
  buffer_list_.push_front(result);
  return new ReadOnlyBuffer(result);
}

void VorbisManager::StartWriteThread() {
  if (!packet.thread_signal.test_and_set()) {
    // the last thread wrapped up while we were waiting
    // but this doesn't impact the result, so why bother
    // reset things, and maybe this will be useful later
  }

  // offchance that our write thread closes on its own
  // in the span between the VM thread seeing that it's open
  // and sending a close signal
  packet.vm_signal.test_and_set();

  if (!run_thread_.load(std::memory_order_acquire)) {
    stb_vorbis_seek_start(audiofile_);
    // thread is NOT already running
    // do everything in here
    run_thread_.store(true, std::memory_order_release);
    auto func = [](std::shared_ptr<FloatBuf> buf) {buf->Clear();};
    EraseOrCallback(func);

    write_thread_ = std::thread(&VorbisManager::WriteThreadFn, this);
    // we have our ThreadPacket for keeping watch (which it will have access to via `this`)
    write_thread_.detach();
    // wait for the write thread to finish setting up
    while (packet.thread_signal.test_and_set());
    // TimeInfo is now valid -- threads can read
    info.SetSampleRate(sample_rate_);
  }
}

void VorbisManager::StopWriteThread() {

  // if it's false, do nothing -- we've already resolved the case
  // if it's true: get ready to wind down
  if (packet.thread_signal.test_and_set()) {
    // thread still open
    packet.vm_signal.clear();
    // notify that we want it to close
    while (packet.thread_signal.test_and_set());
    // wait for it to spin down
    run_thread_.store(false, std::memory_order_release);
  } else if (run_thread_.load(std::memory_order_acquire)) {
    // thread closed on its own
    run_thread_.store(false, std::memory_order_release);
  }
  // signal is false, and thread is marked as closed. This means it cleaned up itself so there's
  // nothing to do.
}

/**
 *  Returns whether or not the thread is running.
 *  True if so, false if not.
 */ 
bool VorbisManager::IsThreadRunning() {
  return run_thread_.load(std::memory_order_acquire);
}

/**
 *  Returns a const pointer to a TimeInfo, a struct which provides functions which assist in
 *  synchronizing read threads with audio playback.
 */ 
const TimeInfo* VorbisManager::GetTimeInfo() {
  return const_cast<const TimeInfo*>(&info);
}

VorbisManager::~VorbisManager() {
  StopWriteThread();
  stb_vorbis_close(audiofile_);
  delete critical_buffer_;
  delete[] read_buffer_;
  // allow our child threads to keep accessing buffers if they want to :)
}

// PRIVATE FUNCTIONS
// TODO: Write the private functions (thread func)

VorbisManager::VorbisManager(int twopow, stb_vorbis* file) : info(), buffer_power_(twopow + 1) {
  stb_vorbis_info info = stb_vorbis_get_info(file);
  channel_count_ = info.channels;
  sample_rate_ = info.sample_rate;
  audiofile_ = file;
  critical_buffer_ = new FloatBuf(twopow, channel_count_);
  read_buffer_ = new float[critical_buffer_->Capacity()];
}

void VorbisManager::WriteThreadFn() {
  // write this
  uint32_t write_threshold = critical_buffer_->Capacity() / 2;
  int samples_read;

  PaError err;

  // todo: move this init call into the VM constructor (we need it for the lifetime,
  //                                                    or even longer)
  err = Pa_Initialize();

  if (err != paNoError) {
    // cleanup
    run_thread_.store(false, std::memory_order_release);
    // eesh
    return;
  }

  // TODO: Should this in fact be volatile???
  std::atomic_flag callback_signal;

  PaStream* stream;
  const int devnum = 6;   // again i think
  const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(devnum);
  PaStreamParameters outParam;

  outParam.channelCount = channel_count_;
  outParam.device = 6;
  outParam.hostApiSpecificStreamInfo = NULL;
  outParam.sampleFormat = paFloat32;
  outParam.suggestedLatency = devinfo->defaultHighOutputLatency; // temporary :)
  // offset timeinfo based on suggestedLatency?

  // NOTE: data passed into stream may have to be volatile?
  // probably everything passed in here should be --
  // given that it can be called at interrupt level

  // i think the bits on threads are bullshit but idk
  // create a small struct which encompasses the crit buffer and the signal
  err = Pa_OpenStream(&stream,
                      NULL,
                      &outParam,
                      sample_rate_,
                      paFramesPerBufferUnspecified,
                      paNoFlag,
                      VorbisManager::PaCallback,
                      reinterpret_cast<void*>(&callback_signal));
  packet.thread_signal.clear();
  if (err != paNoError) {
    // cleanup again
  }

  for (;;) {
    if (!packet.vm_signal.test_and_set()) {
      // kill signal from thread
      break;
    }

    // continue unabated
    if (!PopulateBuffers(write_threshold)) {
      // stream is empty -- done reading
      break;
    }
  }

  // wait for the callback to wind down
  while (callback_signal.test_and_set());
  // callback is done -- killit
  Pa_CloseStream(stream);
  err = Pa_Terminate();
  if (err != paNoError) {
    // something is fucky!
  }
  run_thread_.store(false, std::memory_order_release);
}

void VorbisManager::EraseOrCallback(const std::function<void(std::shared_ptr<FloatBuf>)>& func) {
  // ptr
  std::shared_ptr<AudioBufferSPSC<float>> ptr;
  // lock the list
  std::lock_guard lock(buffer_list_lock_);
  auto itr = buffer_list_.begin();
  while (itr != buffer_list_.end()) {
    // get shared from weak
    ptr = itr->lock();
    // see if it is in fact expired
    if (itr->expired()) {
      // delete it if so
      itr = buffer_list_.erase(itr);
    } else {
      // perform callback otherwise
      func(ptr);
    }
  }
}

// todo: multithread erase-or-callback
// create a thread+queue struct which accepts lambdas and runs them to completion
// send the request to all threads which are not replaced, one by one
// waiting for their completion is a bit finicky
// not right now though
bool VorbisManager::PopulateBuffers(int write_size) {
  if (write_size > critical_buffer_->Capacity()) {
    // something is wrong!
    return false;
    // what the fuck are you doing broh!
  }
  
  // read from vorbis
  int readsize = stb_vorbis_get_samples_float_interleaved(audiofile_, channel_count_, read_buffer_, write_size);
  // spin until space is available
  while (critical_buffer_->GetMaximumWriteSize() < readsize);
  // write to crit buffer
  critical_buffer_->Write(read_buffer_, readsize);
  // call our force callback
  auto callback = [this, readsize](std::shared_ptr<FloatBuf> buf) {
    FillBufferListCallback(buf, read_buffer_, readsize);
  };

  EraseOrCallback(callback);
  // return false if we're at the end (this should be OK, problems may appear :/)
  return (readsize == write_size);
}

void VorbisManager::FillBufferListCallback(std::shared_ptr<FloatBuf> buf, float* input, int write_size) {
  // try to write
  if (!buf->Write(input, write_size)) {
    // if it fails...
    // try to FORCE WRITE
    buf->Force_Write(input, write_size);
  }
}

int VorbisManager::PaCallback(  const void* input,
                                void* output,
                                unsigned long frameCount,
                                const PaStreamCallbackTimeInfo* info,
                                PaStreamCallbackFlags statusFlags,
                                void* userdata  )
{
  CallbackPacket* packet = reinterpret_cast<CallbackPacket*>(userdata);
  float* outputData = reinterpret_cast<float*>(output);
  FloatBuf* buf = packet->buf;
  int samplecount = frameCount * buf->GetChannelCount();
  // try to read
  // if we need to do postprocessing this can be overwritten
  // TODO: could probably get rid of the default read as
  //       this style is the only one we really need but oh well :/

  // also: can return paComplete 


  // lol wrote this because i didnt even *think* it would
  // be necessary to read straight to the buffer what a mongoloid
  if (!buf->ReadToBuffer(samplecount, outputData)) {
    // if it fails, check if the buffer is empty
    if (buf->Empty()) {
      // send zeroes to the buffer
      for (int i = 0; i < samplecount; i++) {
        outputData[i] = 0.0f;
      }
      // call on the write thread to clean up
      packet->callback_signal.clear();
    } else {
      // there's at least some data that we can read
      // read what we can and pad the rest with zeroes
      float* remainingData;
      int samples_read = buf->Peek(samplecount, &remainingData);
      int offset;
      for (offset = 0; offset < samples_read; offset++) {
        outputData[offset] = remainingData[offset];
        // extract everything
      }
      // read zeroes into the rest
      for (; offset < samplecount; offset++) {
        outputData[offset] = 0.0f;
      }
    }
  }

  return 0;
}

