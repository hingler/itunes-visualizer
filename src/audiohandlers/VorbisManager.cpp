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

ReadOnlyBuffer* VorbisManager::CreateBufferInstance(int twopow) {
  if (pow(2, twopow) <= critical_buffer_->Capacity()) {
    return nullptr;
  }

  std::shared_ptr<FloatBuf> result(new FloatBuf(twopow, channel_count_));
  std::lock_guard<std::mutex> lock(buffer_list_lock_);
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
    // wait for the write thread to finish
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
// TODO: Write the private functions (constructor, thread func)

void VorbisManager::EraseOrCallback(const std::function<void(std::shared_ptr<FloatBuf>)>& func) {
  std::shared_ptr<AudioBufferSPSC<float>> ptr;
  std::lock_guard lock(buffer_list_lock_);
  auto itr = buffer_list_.begin();
  while (itr != buffer_list_.end()) {
    ptr = itr->lock();
    if (itr->expired()) {
      itr = buffer_list_.erase(itr);
    } else {
      func(ptr);
    }
  }
}