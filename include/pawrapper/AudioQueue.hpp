/**
 * quick circular buffer intended for reading and storing data
 * from stb_vorbis intended for usage in our wrapper classs
 * 
 * spins up an i/o thread to handle audio decoding
 * while the system reads ahead
 */ 

// spearing your son with a uranium pickaxe

#ifndef AUDIO_QUEUE_H_
#define AUDIO_QUEUE_H_

#include <cinttypes>

template <typename BUFFER_UNIT> // should only be (u) int8, int16, or int32
class AudioQueue {
  private:
    int32_t buffer_capacity_;
    BUFFER_UNIT* buffer_;
    int32_t buffer_start_;
    int32_t buffer_size_;

    int32_t last_read_len_;

    void CheckRep() {

    };
  public:
    AudioQueue(int len) : buffer_capacity_(len), 
                          buffer_(new BUFFER_UNIT[buffer_len_]),
                          last_read_len_(0),
                          buffer_start_(0),
                          buffer_size_(0) { };

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
    bool Write(BUFFER_UNIT* data, int32_t len) {
      if (len + buffer_size_ > buffer_capacity_) {
        // at capacity after writing
        return false;
      }

      int32_t bytes_written = 0;
      int32_t index;

      while (bytes_written < len) {
        index = (buffer_start_ + buffer_size_) % buffer_capacity_;
        buffer_[index] = data[bytes_written];
        bytes_written++;
        buffer_size_++;
      }

      return true;
    }

    /**
     *  Adjusts the capacity of the buffer to store a different number of elements for you
     *  Args:
     *    - newlen, the new length desired for the buffer.
     * 
     *  Returns:
     *    - true if the operation was successful, false otherwise.
     */ 
    // bool Resize(int newlen) {

    // }

    /**
     *  Read the next item in the queue and advance.
     * 
     *  Returns:
     *    - The next item in the queue, if available.
     *    - Otherwise (list is empty), returns NULL.
     */ 
    BUFFER_UNIT Pop() {
      if (buffer_size_ <= 0) {
        return NULL;
      }

      if (buffer_start_ >= buffer_capacity_) {
        buffer_start_ -= buffer_capacity_;
      }

      buffer_size_--;
      return buffer_[buffer_start_++];
    }

    /**
     *  Read the next item in the queue, and do not advance.
     * 
     *  Returns:
     *    - The first item in the queue if available.
     *    - NULL, if list is empty (otherwise).
     */ 
    BUFFER_UNIT Peek() {
      if (buffer_size_ <= 0) {
        return NULL;
      }

      if (buffer_start_ >= buffer_capacity_) {
        buffer_start_ -= buffer_capacity_;
      }

      return buffer_[buffer_start_];
    }

    ~AudioQueue() {

    }

};  // class AudioQueue

#endif  // AUDIO_QUEUE_H_