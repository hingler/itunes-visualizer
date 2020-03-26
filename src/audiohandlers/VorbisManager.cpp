#include "audiohandlers/VorbisManager.hpp"
#include "audiohandlers/AudioBufferSPSC.hpp"

#include "vorbis/stb_vorbis.h"

#include <algorithm>
#include <cmath>

// buffer w/ float template
typedef AudioBufferSPSC<float> FloatBuf;
// shared buffer ptr
typedef std::shared_ptr<FloatBuf> FloatBufPtr;
// used to pass lambda expressions in as callback
typedef std::function<void(FloatBuf*)> BufferCallback;

// TODO: Initialize/Terminate PA, etc etc

VorbisManager::VorbisManager(int twopow) : critical_buffer_capacity_(pow(2, twopow)),
                                           run_thread_(false) {
  // set to null initially
  // this is nonideal but its fine
  // plus its more consistent
  audiofile_ = NULL;
  critical_buffer_ = new FloatBuf(twopow);

  // initialize the channel buffer, expecting to store max number of channels
  channel_buffers_ = new float[channel_count_ * static_cast<int>(pow(2, twopow))];
}

bool VorbisManager::SetFilename(char* filename) {
  if (run_thread_.load(std::memory_order_acquire)) {
    // oop
    return false;
  }

  int error_detect;
  audiofile_ = stb_vorbis_open_filename(filename, &error_detect, NULL);

  if (audiofile_ == NULL) {
    // failed for some reason
    // maybe make an error bit accessible?
    return false;
  }

  stb_vorbis_info info = stb_vorbis_get_info(audiofile_);
  sample_rate_ = info.sample_rate;
  channel_count_ = info.channels;

  info_.SetSampleRate(info.sample_rate);

  return true;
}

FloatBufPtr VorbisManager::CreateBufferInstance(int twopow) {
  if (static_cast<int>(pow(2, twopow)) <= critical_buffer_capacity_) {
    // desired buffer is smaller than critical minimum
    return nullptr;
  }

  // this sounds ok i think
  FloatBufPtr buf_ptr = std::make_shared<FloatBuf>(twopow);
  buffer_list_lock.lock();
  buffer_list_.push_front(std::weak_ptr<FloatBuf>(buf_ptr));
  buffer_list_lock.unlock();

  return buf_ptr;
}

bool VorbisManager::StartWriteThread() {
  // wipe the buffers as well as any others that have been spawned
  if (audiofile_ == NULL) {
    // no file created
    return false;
  }

  if (run_thread_.load(std::memory_order_acquire)) {
    // thread already running
    return false;
  }

  critical_buffer_->Clear();
  auto callback = [](FloatBuf* buf) { buf->Clear(); };
  EraseOrCallback(callback);

  int errint;
  // seek to the start of the file
  errint = stb_vorbis_seek_start(audiofile_);

  if (errint == -1) {
    // i think this is right
    // TODO: print err info
    return false;
  }

  int samples_read = stb_vorbis_get_samples_float_interleaved(audiofile_, channel_count_, channel_buffers_, critical_buffer_capacity_);

  if (samples_read == 0) {
    // file is empty???
    return false;
  }

  // populate everything
  PopulateBuffers(critical_buffer_capacity_);
  // start up write thread, passing the containing class as arg
  run_thread_.store(true, std::memory_order_release);
  write_thread_ = std::thread(&VorbisManager::WriteThreadFn, this);
  return true;
}

bool VorbisManager::StopWriteThread() {
  if (!run_thread_.load(std::memory_order_acquire)) {
    // thread not running
    return false;
  }

  run_thread_.store(std::memory_order_release);
}

bool VorbisManager::PopulateBuffers(uint32_t write_size) {
  // get write size
  if (write_size < critical_buffer_->GetMaximumWriteSize()) {
    return false;
  }

  // this is the only thread so it should be good
  critical_buffer_->Write(channel_buffers_, write_size);
  auto callback = [this, write_size](FloatBuf* buf) { buf->Write(channel_buffers_, write_size); };

  EraseOrCallback(callback);
  return true;
}


void VorbisManager::WriteThreadFn() {
  // initialize, etc. in here
  // attempt to fill the buffer when it drops below 50% capacity
  uint32_t write_threshold = critical_buffer_capacity_ / 2;
  int samples_read;

  PaError err;

  // heap-alloc a timeinfo
  // assign it to the `timeinfo` var
  // no intermediate state

  // excessive calls to Pa_Initialize maybe but it shouldn't be that bad
  err = Pa_Initialize();
  if (err != paNoError) {
    // error!!!
    // terminate prematurely and raise some error flag
    return;
  }

  PaStream* stream;

  const int devnum = 6;
  // just a guess for now -- I need to figure this out
  const PaDeviceInfo* info = Pa_GetDeviceInfo(devnum);
  PaStreamParameters outParam;

  outParam.channelCount = info->maxOutputChannels;
  outParam.device = devnum;
  outParam.hostApiSpecificStreamInfo = NULL;
  outParam.sampleFormat = paFloat32;
  outParam.suggestedLatency = info->defaultLowOutputLatency;

  err = Pa_OpenStream(&stream,
                      NULL,
                      &outParam,
                      sample_rate_,
                      paFramesPerBufferUnspecified,
                      paNoFlag,
                      VorbisManager::PaCallback,
                      NULL);
  
  // create time info
  for (;;) {
    // thread should no longer be running
    if (!run_thread_.load(std::memory_order_acquire)) {
      // stream is over
      Pa_CloseStream(stream);
      // set the callback flag to indicate it is complete
      break;
    }
    // write the next set of samples to the buffer
    samples_read = stb_vorbis_get_samples_float_interleaved(audiofile_, channel_count_, channel_buffers_, write_threshold);
    if (!samples_read) {
      // buffer is exhausted
      break;
    }
    while (critical_buffer_->GetMaximumWriteSize() < write_threshold);
    // its good
    PopulateBuffers(write_threshold);
  }

  // perform some check as to whether the callback has pulled an empty value (is done)
  while (true);

  Pa_CloseStream(stream);

  run_thread_.store(false, std::memory_order_release);
}

// Private Functions
void VorbisManager::EraseOrCallback(BufferCallback) {
  std::shared_ptr<FloatBuf> ptr;
  // lock list
  buffer_list_lock.lock();
  auto itr = buffer_list_.begin();
  while (itr != buffer_list_.end()) {
    ptr = itr->lock();
    if (itr->expired()) {
      // expired -- toss it
      itr = buffer_list_.erase(itr);
    } else {
      // perform callback func
      BufferCallback(itr);
    }
  }
  buffer_list_lock.unlock();
}

VorbisManager::~VorbisManager() {
  StopWriteThread();    // ensure the write thread doesn't continue on without us
  while (true);         // wait for the run_thread_ sign to flip
  if (audiofile_ != NULL) {
    stb_vorbis_close(audiofile_);
  }

  // channel_buffers_ and critical_buffer_ are used by the write thread
  // we cannot use join in most cases, so we are reliant on detach to work correctly
  // note that the write thread will invalidate the timeinfo

  // thus, we can wait for our writethread to complete by looping on IsValid.
  delete channel_buffers_;
  delete critical_buffer_;
}

typedef std::chrono::high_resolution_clock hrc;

