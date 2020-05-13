#ifndef AUDIOBUFFERSPSC_H_
#define AUDIOBUFFERSPSC_H_

#include <atomic>
#include <cstdint>
#include <cmath>
#include <mutex>

#include <iostream>

struct pc_marker {
  uint32_t position; // the index which the marker points to (masked 2x)
  uint32_t safesize;  // minimum number of elements which are guaranteed to be available
};

// default: interleaved
// chunked: separate by channel

// only one should be used -- behavior is undefined if mixing
template <typename BUFFER_UNIT>
class AudioBufferSPSC {
 public:
  AudioBufferSPSC(int twopow, int channel_count = 1) :
    channel_count_(channel_count),
    buffer_capacity_(pow(2, twopow) * channel_count_),
    buffer_(new BUFFER_UNIT[buffer_capacity_]),
    readzone_(new BUFFER_UNIT[buffer_capacity_]),
    channelzone_(new BUFFER_UNIT*[channel_count]),
    reader_thread_({0, 0}),
    shared_read_(0),
    read_marker_(0),
    writer_thread_({0, 0}),
    shared_write_(0)
    {
      writer_thread_.safesize = buffer_capacity_;
    }

  /**
   * Reads a portion of the buffer (if possible) to the readzone, and
   * returns a pointer to it. Does not advance the read-marker.
   * 
   * Arguments:
   *  count, the number of elements to be read.
   *  output, an output parameter which will point to the desired memory.
   * 
   * Returns:
   *  The number of bytes which could be read.
   */ 
  size_t Peek(uint32_t count, BUFFER_UNIT** output) {
    uint32_t len;
    std::lock_guard<std::mutex> lock(read_lock_);
    if (reader_thread_.safesize < count) {
      // no guarantee that we have enough room to read -- check the atomic
      UpdateReaderThread();
    }

    if (reader_thread_.safesize < count) {
      len = reader_thread_.safesize;
    } else {
      len = count;
    }

    if (len == 0) {
      return 0;
    }
    
    uint32_t masked_read = Mask(reader_thread_.position);
    for (uint32_t i = 0; i < len; i++) {
      if (masked_read >= buffer_capacity_) {
        masked_read -= buffer_capacity_;
      }

      readzone_[i] = buffer_[masked_read++];
    }

    *output = readzone_;
    return len;
  }

  /**
   *  Returns samples in a "chunked" format, separating channels into individual buffers.
   *  If used in conjunction with the default "interleaved" formats, behavior is undefined.
   * 
   *  Arguments:
   *    - framecount, the number of samples read from each channel.
   *    - output, a float pointer array passed in by the client. Must have sufficient space
   *      necessary to contain at least (channel_count_) items.
   *  Returns:
   *    - The number of frames read.
   */ 
  size_t Peek_Chunked(uint32_t framecount, BUFFER_UNIT*** output) {
    uint32_t len;
    uint32_t count = framecount * channel_count_;

    RefreshChannelZone();

    std::lock_guard<std::mutex> lock(read_lock_);
    if (reader_thread_.safesize < count) {
      UpdateReaderThread();
    }

    if (reader_thread_.safesize < count) {
      len = reader_thread_.safesize / channel_count_;
    } else {
      len = framecount;
    }

    uint32_t masked_read = Mask(reader_thread_.position);

    for (uint32_t i = 0; i < len; i++) {
      for (int j = 0; j < channel_count_; j++) {
        if (masked_read >= buffer_capacity_) {
          masked_read -= buffer_capacity_;
        }
        channelzone_[j][i] = buffer_[masked_read++];
      }
    }

    *output = channelzone_;

    return len;
  }

  /**
   *  Skips some number of terms which are being read. Does not increment
   *  number of samples read.
   */ 
  bool Skip(uint32_t count) {
    std::lock_guard<std::mutex> lock(read_lock_);
    if (reader_thread_.safesize < count) {
      UpdateReaderThread();
    }

    if (reader_thread_.safesize < count) {
      return false;
    }

    reader_thread_.position = MaskTwo(reader_thread_.position + count);
    reader_thread_.safesize -= count;

    shared_read_.store(reader_thread_.position, std::memory_order_release);
  }

  bool Skip_Chunked(uint32_t framecount) {
    return Skip(framecount * channel_count_);
  }

  /**
   * Attempt to read from the buffer and advance the cursor.
   * Returns a pointer if the memory requested exists, otherwise
   * returns a nullptr.
   * 
   * In the event of a multichannel format, samples are returned in the
   * order they were inserted.
   * 
   * Arguments:
   *  - count, the number of elements we are attempting to read.
   * 
   * Returns:
   *  - a R/W pointer to the read data.
   */
  BUFFER_UNIT* Read(uint32_t count) {
    std::lock_guard<std::mutex> lock(read_lock_);
    if (reader_thread_.safesize < count) {
      UpdateReaderThread();

      if (reader_thread_.safesize < count) {
        return nullptr; 
      }
    }

    uint32_t masked_read = Mask(reader_thread_.position);
    for (uint32_t i = 0; i < count; i++) {
      if (masked_read >= buffer_capacity_) {
        masked_read -= buffer_capacity_;
      }
      readzone_[i] = buffer_[masked_read++];
    }

    reader_thread_.position = MaskTwo(reader_thread_.position + count);
    reader_thread_.safesize -= count;

    shared_read_.store(reader_thread_.position, std::memory_order_release);
    read_marker_.fetch_add(count, std::memory_order_acq_rel);

    return readzone_;
  }

  /**
   *  Instead of using the internal buffer, reads directly to a provided
   *  output buffer. Returns whether or not the read option
   *  was successful.
   * 
   *  @param count - the number of samples we are reading from the buffer.
   *  @param output - the buffer we're outputting to.
   *  @param output_channel_count - the number of channels in the output.
   *                                used to play mono sound on stereo output, for instance
   */ 
  bool ReadToBuffer(uint32_t count, BUFFER_UNIT* output, int output_channel_count) {
    std::lock_guard<std::mutex> lock(read_lock_);
    if (reader_thread_.safesize < (count * output_channel_count)) {
      UpdateReaderThread();

      if (reader_thread_.safesize < count * output_channel_count) {
        return false; 
      }
    }

    int sample_channel_ratio = output_channel_count / channel_count_;

    uint32_t masked_read = Mask(reader_thread_.position);
    for (uint32_t i = 0; i < count; i++) {
      if (masked_read >= buffer_capacity_) {
        masked_read -= buffer_capacity_;
      }
      for (int j = 0; j < sample_channel_ratio; j++) {
        output[i * sample_channel_ratio + j] = buffer_[masked_read];
      }

      masked_read++;     
    }

    reader_thread_.position = MaskTwo(reader_thread_.position + count);
    reader_thread_.safesize -= count;

    shared_read_.store(reader_thread_.position, std::memory_order_release);
    read_marker_.fetch_add(count, std::memory_order_acq_rel);

    return true;
  }

  BUFFER_UNIT** Read_Chunked(uint32_t framecount) {
    uint32_t count = framecount * channel_count_;
    std::lock_guard<std::mutex> lock(read_lock_);
    if (reader_thread_.safesize < count) {
      UpdateReaderThread();
    }

    if (reader_thread_.safesize < count) {
      return nullptr;
    }

    RefreshChannelZone();

    uint32_t masked_read = Mask(reader_thread_.position);
    for (uint32_t i = 0; i < framecount; i++) {
      for (int j = 0; j < channel_count_; j++) {
        if (masked_read >= buffer_capacity_) {
          masked_read -= buffer_capacity_;
        }
        channelzone_[j][i] = buffer_[masked_read++];
      }
    }

    reader_thread_.position = MaskTwo(reader_thread_.position + count);
    reader_thread_.safesize -= count;

    shared_read_.store(reader_thread_.position, std::memory_order_release);
    read_marker_.fetch_add(count, std::memory_order_acq_rel);

    return channelzone_;
  }

  /**
   *  Synchronizes the audio buffer to a given sample number.
   *  Note: buffer is emptied if sample_num is larget than the number of entries currently contained
   *  Adjusts the internal sample counter to the provided value.
   */ 
  void Synchronize(uint32_t sample_num) {
    std::lock_guard<std::mutex> lock(read_lock_);
    // take into account cases where the sample point is behind current
    uint32_t count = sample_num - read_marker_.load(std::memory_order_acquire);
    // ensure that we have enough space to leap
    if (count > 0) {
      if (count > reader_thread_.safesize) {
        UpdateReaderThread();
      }

      // wipes the buffer if necessary
      int offset = Min(reader_thread_.safesize, count);

      reader_thread_.safesize -= offset;
      reader_thread_.position = MaskTwo(reader_thread_.position + offset);
      shared_read_.store(reader_thread_.position, std::memory_order_release);
      read_marker_.store(sample_num, std::memory_order_release);
    }
  }

  void Synchronize_Chunked(uint32_t frame_num) {
    Synchronize(frame_num * channel_count_);
  }

  /**
   * Write to the buffer.
   * Returns whether or not the write was successful --
   * false if not enough room, true otherwise.
   * 
   * If mixing this write call with other chunked calls,
   * entries must be interleaved, with a series of n samples
   * containing each sample for all n channels in a frame.
   */ 
  bool Write(const BUFFER_UNIT* data, uint32_t count) {
    std::lock_guard<std::mutex> lock(write_lock_);
    if (writer_thread_.safesize < count) {
      UpdateWriterThread();
    }

    if (writer_thread_.safesize < count) {
      return false;
    }

    uint32_t masked_write = Mask(writer_thread_.position);
    for (uint32_t i = 0; i < count; i++) {
      if (masked_write >= buffer_capacity_) {
        masked_write -= buffer_capacity_;
      }
      buffer_[masked_write++] = data[i];
    }

    writer_thread_.position = MaskTwo(writer_thread_.position + count);
    writer_thread_.safesize -= count;

    shared_write_.store(writer_thread_.position, std::memory_order_release);

    return true;
  }

  void Force_Write(const BUFFER_UNIT* data, uint32_t count) {
    std::scoped_lock lock(read_lock_, write_lock_);
    UpdateWriterThread();

    if (writer_thread_.safesize < count) {
      // need to adjust the write thread
      int to_skip = count - writer_thread_.safesize;
      reader_thread_.position = MaskTwo(reader_thread_.position + to_skip);
      reader_thread_.safesize -= to_skip;
      writer_thread_.safesize += to_skip;
      shared_read_.store(reader_thread_.position, std::memory_order_release);
    }

    uint32_t masked_write = Mask(writer_thread_.position);
    for (uint32_t i = 0; i < count; i++) {
      if (masked_write >= buffer_capacity_) {
        masked_write -= buffer_capacity_;
      }
      buffer_[masked_write++] = data[i];
    }

    writer_thread_.position = MaskTwo(writer_thread_.position + count);
    writer_thread_.safesize -= count;

    shared_write_.store(writer_thread_.position, std::memory_order_release);
  }

  /**
   *  Wipes the contents of the queue. Not thread safe.
   */ 
  void Clear() {
    std::scoped_lock lock(read_lock_, write_lock_);
    writer_thread_.position = 0;
    reader_thread_.position = 0;
    writer_thread_.safesize = buffer_capacity_;
    reader_thread_.safesize = 0;
    shared_read_.store(0);
    shared_write_.store(0);
    read_marker_.store(0);    // wipe history as well
  }

  uint32_t GetMaximumWriteSize() {
    std::lock_guard<std::mutex> lock(write_lock_);
    UpdateWriterThread();
    return writer_thread_.safesize;
  }

  uint32_t Size() const {
    uint32_t size = shared_write_.load(std::memory_order_acquire);
    size -= shared_read_.load(std::memory_order_acquire);
    return MaskInclusive(size);
  }

  uint32_t Empty() const {
    uint32_t write = shared_write_.load(std::memory_order_acquire);
    return (write == shared_read_.load(std::memory_order_acquire));
  }

  uint32_t Capacity() const {
    return buffer_capacity_;
  }

  uint32_t GetItemsRead() const {
    return read_marker_.load(std::memory_order_acq_rel);
  }

  int GetChannelCount() const {
    return channel_count_;
  }

  ~AudioBufferSPSC() {
    delete[] buffer_;
    delete[] readzone_;
    delete[] channelzone_;
  }

  void operator=(const AudioBufferSPSC& other) = delete;
  AudioBufferSPSC(const AudioBufferSPSC &) = delete;

 private:
  const int channel_count_; // number of channels -- used for synchronization of r/w ops
  
  const uint32_t buffer_capacity_;  // max capacity of the buffer
  BUFFER_UNIT* buffer_;   // pointer to internal buffer

  BUFFER_UNIT* readzone_;   // read space which can be read/modified by read thread
  BUFFER_UNIT** channelzone_; // space allocated for per-channel pointers

  pc_marker reader_thread_;
  std::atomic_uint32_t shared_read_;  // shared ptr for syncing read val
  std::atomic_uint64_t read_marker_;  // tracks number of samples read thus far
  std::mutex read_lock_;
  
  char CACHE_BUSTER[64];  // yall is playn -_-

  pc_marker writer_thread_;
  std::atomic_uint32_t shared_write_;   // shared ptr for syncing write val
  std::mutex write_lock_;

  uint32_t Mask(uint32_t input) const {
    return input & (buffer_capacity_ - 1);
  }

  uint32_t MaskInclusive(uint32_t input) const {
    uint32_t masked_input = Mask(input);
    if (input != 0 && masked_input == 0) {
      // multiple of capacity
      return buffer_capacity_;
    }

    return masked_input;
  }

  /**
   * Mask the input to a range of 2 * the buffer length.
   * 
   * Arguments:
   *  - input, the value we are masking.
   * 
   * Returns:
   *  - masked value
   */ 
  uint32_t MaskTwo(uint32_t input) const {
    return input & ((buffer_capacity_ << 1) - 1);
  }

  void UpdateReaderThread() {
    uint32_t pos = shared_write_.load(std::memory_order_acquire);
    reader_thread_.safesize = MaskInclusive(pos - reader_thread_.position);
  }

  void UpdateWriterThread() {
    uint32_t pos = shared_read_.load(std::memory_order_acquire);
    writer_thread_.safesize = buffer_capacity_ - MaskInclusive(writer_thread_.position - pos);
  }
  
  void RefreshChannelZone() {
    int per_channel_capacity = (buffer_capacity_ / channel_count_);
    for (int i = 0; i < channel_count_; i++) {
      channelzone_[i] = readzone_ + (i * per_channel_capacity);
    }
  }

  inline uint32_t Min(uint32_t a, uint32_t b) {
    return (a < b ? a : b);
  }
};  // class AudioBufferSPSC

#endif  // AUDIOBUFFERSPSC_H_