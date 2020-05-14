#include "audiohandlers/VorbisManager.hpp"
#include <iostream>
#include <string>

typedef AudioBufferSPSC<float> FloatBuf;

// TIMEINFO CODE

TimeInfo::TimeInfo() : sample_rate_(0),
                       playback_epoch_(std::chrono::high_resolution_clock::now()) { }
                      
TimeInfo::TimeInfo(int sample_rate) : sample_rate_(sample_rate),
                                      playback_epoch_(std::chrono::high_resolution_clock::now()) {}

int TimeInfo::GetCurrentSample() const {
  std::shared_lock<std::shared_mutex> lock(info_lock_);
  if (sample_rate_ == 0) {
    return -1;
  }

  std::chrono::duration<double, std::milli> offset =
    std::chrono::high_resolution_clock::now() - playback_epoch_;
  return static_cast<int>((sample_rate_ * offset.count()) / 1000);
}

bool TimeInfo::IsThreadRunning() const {
  std::shared_lock lock(info_lock_);
  return !(sample_rate_ <= 0);
}

void TimeInfo::SetSampleRate(int sample_rate) {
  std::lock_guard<std::shared_mutex> lock(info_lock_);
  sample_rate_ = sample_rate;
}

void TimeInfo::ResetEpoch() {
  std::lock_guard<std::shared_mutex> lock(info_lock_);
  playback_epoch_ = std::chrono::high_resolution_clock::now();
}

// READONLYBUFFER CODE
ReadOnlyBuffer::ReadOnlyBuffer(std::shared_ptr<AudioBufferSPSC<float>> buffer, 
                               const TimeInfo* info) : info_(info), buffer_(buffer) {}

size_t ReadOnlyBuffer::Peek_Chunked(uint32_t framecount, float*** output) {
  return buffer_->Peek_Chunked(framecount, output);
}

float** ReadOnlyBuffer::Read_Chunked(uint32_t framecount) {
  return buffer_->Read_Chunked(framecount);
}

int ReadOnlyBuffer::Synchronize_Chunked() {
  return Synchronize_Chunked(0);
}

int ReadOnlyBuffer::Synchronize_Chunked(float offset) {
  int samplenum = info_->GetCurrentSample() + (offset * info_->sample_rate_);
  if (samplenum < 0) {
    samplenum = 0;
    if (info_->GetCurrentSample() == -1) {
      return -1;
    }
  }

  buffer_->Synchronize_Chunked(samplenum);
  return samplenum;
}

int ReadOnlyBuffer::Size() {
  return buffer_->Size();
}

ReadOnlyBuffer::~ReadOnlyBuffer() { 
}

// VORBISMANAGER CODE

VorbisManager* VorbisManager::GetVorbisManager(int twopow, AudioReader* reader) {


  if (twopow <= 1) {
    // unreasonable
    return nullptr;
  }
  int err;

  err = Pa_GetDeviceCount();

  if (err < 0) {
    // failed to initialize PA
    return nullptr;
  }

  return new VorbisManager(twopow, reader);
}

ReadOnlyBuffer* VorbisManager::CreateBufferInstance() {
  std::shared_ptr<FloatBuf> result(new FloatBuf(buffer_power_, channel_count_));
  std::lock_guard lock(buffer_list_lock_);
  buffer_list_.push_front(result);
  
  return new ReadOnlyBuffer(result, const_cast<const TimeInfo*>(&info));
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
    reader_->Seek(0);
    // thread is NOT already running
    // do everything in here
    run_thread_.store(true, std::memory_order_release);
    auto func = [](std::shared_ptr<FloatBuf> buf) {buf->Clear();};
    EraseOrCallback(func);
    write_thread_ = std::thread(&VorbisManager::WriteThreadFn, this);
    write_thread_.detach();
    // wait for the write thread to finish setting up
    while (packet.thread_signal.test_and_set());
    info.SetSampleRate(sample_rate_);
  }
}

void VorbisManager::StopWriteThread() {
  if (packet.thread_signal.test_and_set()) {
    packet.vm_signal.clear();
    while (packet.thread_signal.test_and_set()) {
      // take it easy
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    run_thread_.store(false, std::memory_order_release);
  } else if (run_thread_.load(std::memory_order_acquire)) {
    // thread closed on its own
    run_thread_.store(false, std::memory_order_release);
  }
}

void VorbisManager::ThreadWait() {
  while (run_thread_.load(std::memory_order_acquire));
}

bool VorbisManager::IsThreadRunning() {
  return run_thread_.load(std::memory_order_acquire);
}

VorbisManager::~VorbisManager() {
  if (run_thread_.load(std::memory_order_acquire)) {
    StopWriteThread();
  }

  delete critical_buffer_;
  delete[] read_buffer_;
  delete reader_;
  // allow our child threads to keep accessing buffers if they want to :)
}

// PRIVATE FUNCTIONS

VorbisManager::VorbisManager(int twopow, AudioReader* reader) : run_thread_(false), 
                                                                buffer_power_(twopow + 1), info()
                                                                {
  // stores the number of channels on the file
  // we want to have a marker of the number of channels on the output
  channel_count_ = reader->GetChannelCount();
  sample_rate_ = reader->GetSampleRate();
  reader_ = reader;
  
  critical_buffer_ = new FloatBuf(twopow, channel_count_);
  read_buffer_ = new float[critical_buffer_->Capacity()];
}

// todo: make bool :/
void VorbisManager::WriteThreadFn() {
  uint32_t write_threshold = critical_buffer_->Capacity() / 2;

  PaError err;

  // TODO: Should this in fact be volatile???

  PaStream* stream;
  
  // feel like this could be called within StartWriteThread but either/or i guess
  PopulateBuffers(critical_buffer_->Capacity());

  CallbackPacket* callback_packet = new CallbackPacket();
  callback_packet->buf = critical_buffer_;
  callback_packet->callback_signal.test_and_set();

  err = Pa_OpenDefaultStream(&stream,
                      0,
                      2,
                      paFloat32,
                      sample_rate_,
                      paFramesPerBufferUnspecified,
                      VorbisManager::PaCallback,
                      reinterpret_cast<void*>(callback_packet));
  Pa_StartStream(stream);
  // todo: take latency into account
  info.ResetEpoch();
  packet.thread_signal.clear();
  if (err != paNoError) {
    // cleanup again
  }

  for (;;) {
    if (!packet.vm_signal.test_and_set()) {
      break;
    }
    if (!PopulateBuffers(write_threshold)) {
      // stream is empty -- done reading
      break;
    }
  }

  // wait for the callback to wind down
  while (callback_packet->callback_signal.test_and_set());
  // callback is done -- killit
  Pa_CloseStream(stream);
  err = Pa_Terminate();
  if (err != paNoError) {
    // something is fucky!
  }

  run_thread_.store(false, std::memory_order_release);
  packet.thread_signal.clear();
  delete callback_packet;
}

void VorbisManager::EraseOrCallback(const std::function<void(std::shared_ptr<FloatBuf>)>& func) {
  std::shared_ptr<AudioBufferSPSC<float>> ptr;
  std::lock_guard lock(buffer_list_lock_);
  auto itr = buffer_list_.begin();
  while (itr != buffer_list_.end()) {
    ptr = itr->lock();
    // see if it is in fact expired
    if (itr->expired()) {
      // delete it if so
      itr = buffer_list_.erase(itr);
    } else {
      func(ptr);
    }
    itr++;
  }
}

bool VorbisManager::PopulateBuffers(unsigned int write_size) {
  if (write_size > critical_buffer_->Capacity()) {
    // something is wrong!
    return false;
    // what the fuck are you doing broh!
  }

  int readsize = reader_->GetSamplesInterleaved(write_size, read_buffer_);
  // spin until space is available
  while (critical_buffer_->GetMaximumWriteSize() < write_size) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  // todo: this takes up a good chunk of cpu resources afaik
  // telling the buffer to sleep a bit before checking this might be best

  int writesize = (readsize * channel_count_);
  auto callback = [this, writesize](std::shared_ptr<FloatBuf> buf) {
    FillBufferListCallback(buf, read_buffer_, writesize);
  };

  EraseOrCallback(callback);
  critical_buffer_->Write(read_buffer_, writesize);



  return ((write_size / channel_count_) == readsize);
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
  float* output_data = reinterpret_cast<float*>(output);
  FloatBuf* buf = packet->buf;
  size_t samplecount = frameCount * (buf->GetChannelCount());
  // ensure that this all matches up -- if input only has one channel then do something else
  if (!buf->ReadToBuffer(samplecount, output_data, 2)) {
    if (buf->Empty()) {
      // send zeroes to the buffer
      for (size_t i = 0; i < samplecount; i++) {
        output_data[i] = 0.0f;
      }
      packet->callback_signal.clear();
    } else {
      // there's at least some data that we can read
      // read what we can and pad the rest with zeroes
      float* remaining_data;
      size_t samples_read = buf->Peek(samplecount, &remaining_data);
      size_t offset;

      // assumption that audio file has fewer channels than output
      int sample_channel_ratio = 2 / buf->GetChannelCount();
      std::cout << sample_channel_ratio;
      
      for (offset = 0; offset < samples_read; offset++) { 
        for (int i = 0; i < sample_channel_ratio; i++) {
          output_data[sample_channel_ratio * offset + i] = remaining_data[offset];
        }
      }

      for (; offset < samplecount; offset++) {
        output_data[offset] = 0.0f;
      }

      packet->callback_signal.clear();
    }
  }

  return 0;
}

