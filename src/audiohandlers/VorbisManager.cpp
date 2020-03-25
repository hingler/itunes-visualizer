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

VorbisManager::VorbisManager(int twopow) : critical_buffer_capacity_(pow(2, twopow)),
                                                           buffer_list_() {
  
  // set to null initially
  // this is nonideal but its fine
  // plus its more consistent
  audiofile_ = NULL;
  critical_buffer_ = new FloatBuf(twopow);

  // initialize the channel buffer, expecting to store max number of channels
  channel_buffers_[channel_count_ * static_cast<int>(pow(2, twopow))];
}

bool VorbisManager::SetFilename(char* filename) {
  // if this errors out, how do we handle it?
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
}

FloatBufPtr VorbisManager::CreateBufferInstance(int twopow) {
  if (twopow <= critical_buffer_capacity_) {
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

void VorbisManager::StartWriteThread() {

  // TODO: ADD ERROR CHECKING ON ALL OF THESE CALLS!!!

  // wipe the buffers as well as any others that have been spawned
  if (audiofile_ == NULL) {
    // no file created
    return;
  }
  critical_buffer_->Clear();
  auto callback = [](FloatBuf* buf) { buf->Clear(); };
  EraseOrCallback(callback);

  // seek to the start of the file
  stb_vorbis_seek_start(audiofile_);

  int samples_read = stb_vorbis_get_samples_float_interleaved(audiofile_, channel_count_, channel_buffers_, critical_buffer_capacity_);

  PopulateBuffers(critical_buffer_capacity_);

  // start up write thread, passing the containing class as arg
  write_thread_ = std::thread(&VorbisManager::WriteThreadFn, this);
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
  // attempt to fill the buffer when it drops below 50% capacity
  uint32_t write_threshold = critical_buffer_capacity_ / 2;
  int samples_read;
  for (;;) {
    // write the next set of samples to the buffer
    samples_read = stb_vorbis_get_samples_float_interleaved(audiofile_, channel_count_, channel_buffers_, write_threshold);
    if (!samples_read) {
      // buffer is exhausted
      break;
    }
    while (critical_buffer_->GetMaximumWriteSize() < write_threshold);
    // its good
    VorbisManager::PopulateBuffers(write_threshold);
  }

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
  stb_vorbis_close(audiofile_);
}

// TimeInfo

