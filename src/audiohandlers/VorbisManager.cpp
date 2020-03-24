#include "audiohandlers/VorbisManager.hpp"
#include "audiohandlers/AudioBufferSPSC.hpp"

#include "vorbis/stb_vorbis.h"

#include <algorithm>
#include <cmath>

typedef AudioBufferSPSC<float> FloatBuf;
typedef std::shared_ptr<FloatBuf> FloatBufPtr;

bool BufferIsExpired(std::weak_ptr<FloatBuf>);

VorbisManager::VorbisManager(char* filename, int twopow) : critical_buffer_capacity_(twopow),
                                                           buffer_list_() {
  int error_detect;

  audiofile_ = stb_vorbis_open_filename(filename, &error_detect, NULL);

  stb_vorbis_info info = stb_vorbis_get_info(audiofile_);
  sample_rate_ = info.sample_rate;
  channel_count_ = info.channels;

  critical_buffer_ = new FloatBuf(twopow);

  // initialize the channel buffer, expecting to store max number of channels
  channel_buffers_[channel_count_ * static_cast<int>(pow(2, twopow))];
}

FloatBufPtr VorbisManager::CreateBufferInstance(int twopow) {
  if (twopow <= critical_buffer_capacity_) {
    // desired buffer is smaller than critical minimum
    return nullptr;
  }

  FloatBufPtr buf_ptr(new FloatBuf(twopow));
  buffer_list_lock.lock();
  buffer_list_.push_front(std::weak_ptr<FloatBuf>(buf_ptr));
  buffer_list_lock.unlock();

  return buf_ptr;
}

void VorbisManager::PopulateBuffers(uint32_t write_size) {
  stb_vorbis_get_samples_float_interleaved(audiofile_, channel_count_, channel_buffers_, write_size);

  // this is the only thread so it should be good
  critical_buffer_->Write(channel_buffers_, write_size);

  std::shared_ptr<FloatBuf> ptr;
  buffer_list_lock.lock();
  auto itr = buffer_list_.begin();
  while (itr != buffer_list_.end()) {
    ptr = itr->lock();
    if (itr->expired()) {
      // ptr is invalid
      itr = buffer_list_.erase(itr);
    } else {
      // ptr is valid
      ptr->Write(channel_buffers_, write_size);
    }
  }
}

void VorbisManager::WriteThreadFn(void* vorbis_inst) {
  // check to see if there's room in the critical buffer
  // the vorbis callback should be used to extract some playback data
  for (;;) {
    uint32_t bytes_remaining = critical_buffer_->GetMaximumWriteSize();
    if (bytes_remaining > 0) {
      // inconsistent but want to make sure this is carried over properly
      this->PopulateBuffers(bytes_remaining);
    }
  }
}

// callback has a data function

// we can use it to estimate where we are



// TODO: maybe write some portaudio tests elsewhere and check it out
// TODO: maybe do that instead of this to get a feel for how the problem solving will go



// simple synchronization class* which links a spawned buffer to the parent VM
// the VM retains control in this case by creating a per-buffer lock

// critical buffer reads in from file to full capacity
// other buffers are filled in equivalently

// the wrapper class should provide some means of ensuring that we have read to the
// same point as the manager

// for instance, we could have a function, Synchronize(), which could be called (for instance)
// before reading a section

// furthermore, a delay could be taken into account which, given the sample rate and delay,
// attempts to align things a bit better

// a single thread maintains read control -- a "synchronize" call should update the buffer

// note: our critical buffer is read in chunks -- we should go by the sample rate rather than
// its state, but use the state of the critical buffer as a reference point i.e. if we desync
// considerably, something is clearly awry

// in this case we would need to wrap the functionality of the buffer
// to keep track of how far it's read


VorbisManager::~VorbisManager() {
  StopWriteThread();    // ensure the write thread doesn't continue on without us
  stb_vorbis_close(audiofile_);
}

// for ClearFreedBuffers
bool BufferIsExpired(std::weak_ptr<FloatBuf> buf) {
  return buf.expired();
}

