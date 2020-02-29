/**
 * quick circular buffer intended for reading and storing data
 * from stb_vorbis intended for usage in our wrapper classs
 * 
 * spins up an i/o thread to handle audio decoding
 * while the system reads ahead
 * 
 * absolete :)
 */ 

// spearing your son with a uranium pickaxe

#ifndef AUDIO_QUEUE_H_
#define AUDIO_QUEUE_H_

#include <atomic>
#include <cinttypes>

// values are preserved at 2x scale
// if they are the same, our buffer is empty
// if their masks are the same but they are different, our buffer is full
// full buffer: 
//  - write > read, diff
//  - else, add 2xcapac to write + diff

// acquire/release go in tandem
// relaxed carries no order guarantees and just ensures that the operation happens

// threadsafe for single-read/single-write

// some reference for memory ordering:
// https://pdfs.semanticscholar.org/8c15/b938e6fdbcbab6e1b5704857043e5c680734.pdf

// class is intended for an R/W pair, as such multithreading is limited.


// this is hopefully fine?
template <typename BUFFER_UNIT> // should only be (u) int8, int16, or int32
class AudioQueue {
 private:
  const uint32_t buffer_capacity_;
  BUFFER_UNIT* buffer_;
  // todo: figure out atomic mem mgmt shit
  std::atomic_uint32_t buffer_read_;
  std::atomic_uint32_t buffer_write_;

  const uint32_t readsize_;

  std::atomic<BUFFER_UNIT*> read_buffer_;

  std::atomic<BUFFER_UNIT*> aux_buffer_;

  std::atomic_flag buffer_in_use_;

  uint32_t Mask(uint32_t val) {
    return val % buffer_capacity_;
  }

  uint32_t ConstrainParam(uint32_t param) {
    if (param >= 2 * buffer_capacity_) {
      param -= 2 * buffer_capacity_;
    }

    return param;
  }

  uint32_t GetSize(uint32_t rb, uint32_t wb) {
    if (rb > wb) {
      wb += 2 * buffer_capacity_;
    }

    return wb - rb;
  }

 public:
  AudioQueue(int capacity, int readsize) : buffer_capacity_(capacity), 
                        buffer_(new BUFFER_UNIT[buffer_capacity_]),
                        buffer_read_(0),
                        buffer_write_(0),
                        readsize_(readsize),
                        read_buffer_(new BUFFER_UNIT[readsize_]) { };

  /**
   *  Writes `len` of `BUFFER_UNIT` to the queue.
   * 
   *  Arguments:
   *    - data, the data we are intending to write to our buffer.
   *    - len, the length of `data`.
   * 
   *  Returns:
   *    - true if data fits in queue, false otherwise.
   */ 
  bool Write(BUFFER_UNIT* data, uint32_t len) {

    uint32_t read_copy = buffer_read_.load(std::memory_order_acq_rel);
    uint32_t write_copy = buffer_write_.load(std::memory_order_relaxed);

    if (GetSize(read_copy, write_copy) > buffer_capacity_) {
      return false;
    }

    uint32_t masked_write;

    for (uint32_t i = 0; i < len; i++) {
      masked_write = Mask(write_copy);
      buffer_[masked_write] = data[i];
      write_copy++;
    }

    buffer_write_.store(ConstrainParam(write_copy), std::memory_order_release);
    return true;
  }

  /**
   *  Read the next item in the queue and advance.
   * 
   *  Arguments:
   *    success, ref argument set to true if successful and false otherwise.
   * 
   *  Returns:
   *    - The next item in the queue, if available.
   *    - Otherwise (list is empty), returns -1.
   */ 
  BUFFER_UNIT Pop(bool& success) {
    // get a lower bound estimate on the content we have
    uint32_t write_copy = buffer_write_.load(std::memory_order_acquire);
    uint32_t read_copy = buffer_read_.load(std::memory_order_relaxed);

    if (read_copy == write_copy) {
      success = false;
      return -1;
    }
    success = true;
    BUFFER_UNIT target = buffer_[Mask(read_copy++)];
    buffer_read_.store(ConstrainParam(read_copy), std::memory_order_release);
    return target;
  }

  BUFFER_UNIT Pop() {
    // eh
    bool ignore = false;
    return Pop(ignore);
  }

  const BUFFER_UNIT* FillReadBuffer(int32_t len = readsize_) {
    uint32_t write_copy = buffer_read_.load(std::memory_order_relaxed);
    uint32_t read_copy = buffer_write_.load(std::memory_order_acquire);

    if (len > readsize_ || GetSize(read_copy, write_copy) < len) {
      // invalid input value
      return nullptr;
    }

    uint32_t masked_read;

    // if something else is copying this buffer, swap to a new one and flip the read bit.
  if (buffer_in_use_.test_and_set(std::memory_order_acq_rel)) {
    BUFFER_UNIT* main = read_buffer_.load(std::memory_order_)
  }


    for (uint32_t i = 0; i < readsize_; i++) {
      masked_read = Mask(read_copy);
      read_buffer_[i] = buffer_[masked_read];
    }

    return const_cast<const BUFFER_UNIT*>(read_buffer_);
  }

  BUFFER_UNIT* CopyReadBuffer() {
    BUFFER_UNIT* read_buffer = read_buffer_.load(std::memory_order_acq_rel);
    buffer_in_use_.test_and_set(std::memory_order_acq_rel);

    // do your shit
    // wait around until the atomic flag is freed
    // once it is, uh
    // do whatever i guess?
  }

  /**
   *  Read the next item in the queue, and do not advance.
   * 
   *  Returns:
   *    - The first item in the queue if available.
   *    - NULL, if list is empty (otherwise).
   */ 
  BUFFER_UNIT Peek() {
    return buffer_[Mask(buffer_read_.load(std::memory_order_relaxed))];
  }



  uint32_t Size() {
    // generate lock here
    if (buffer_write_ >= buffer_read_) {
      return buffer_write_ - buffer_read_;
    } else {
      return (buffer_write_ + 2 * buffer_capacity_ - buffer_read_);
    }
    // release lock here
  }

  bool Empty() {
    return (buffer_read_ == buffer_write_);
  }

  ~AudioQueue() {
    delete[] buffer_;
  }

};  // class AudioQueue

#endif  // AUDIO_QUEUE_H_