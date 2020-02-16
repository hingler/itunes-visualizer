

// tha idea

// use two structs to keep track of a "minimum guaranteed" access size

// reference: http://daugaard.org/blog/writing-a-fast-and-versatile-spsc-ring-buffer/

// spsc:
//  - on their own, the producer and consumer should have the resources necessary
//    to read/write freely to some allowable extent.
//
//  - when our reader runs dry, we want to check a locked structure to verify its data.
//    then continue on as normal.

// for the callback, need a prealloc'd structure to read out to
// also need some exterior "data" parameter where we can put that shit so that
// this does necessitate a read "lock" so that we can get the data out in one piece
// but the procedure should be quick

// alternative:
// if the other thread has stale data, could we justre bake in a fallback method?
// i.e. attempt to fetch from the read buffer, then just pop the data if not possible?


#include <atomic>
#include <cstdint>
#include <cmath>

struct pc_marker {
  pc_marker(int size) : position(0), safesize(size) { }
  uint32_t position; // the index which the marker points to
  uint32_t safesize;  // space guaranteed ahead of position
};

template <typename BUFFER_UNIT>
class AudioBufferSPSC {

 public:
  AudioBufferSPSC(int twopow, int maxreadlen) :
    buffer_capacity_(pow(2, twopow)),
    buffer_(new BUFFER_UNIT[buffer_capacity_]),
    shared_read_(0),
    shared_write_(0),


    reader_thread(pc_marker(readsize)),
    writer_thread(pc_marker(readsize))
  {
    
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
   *  A read-only pointer to the relevant section of memory.
   *  If the requested amount of memory is unavailable, returns
   *  however many bytes could possibly be copied.
   *  
   */ 
  size_t peek(uint32_t count, const BUFFER_UNIT*& output) {
    // if we don't need to jump, we can just return the buffer chunk
    uint32_t len;
    if (reader_thread_.safesize < count) {

    }
  }
  
 private:
  const uint32_t buffer_capacity_;  // max capacity of the buffer
  BUFFER_UNIT* buffer_;   // pointer to internal buffer

  std::atomic_uint32_t shared_read_;  // shared ptr for syncing read val
  std::atomic_uint32_t shared_write_;   // shared ptr for syncing write val

  // todo: resolve issue of reading data while it may be written at the same time
  // in the read thread: read to a second ring buffer which can be shared with a render thread.
  // even better: if the goal is an fft printout, we can rely on an atomic int to track sample count,
  // and read a set size from that ring buffer, only advancing when necessary

  // a safe readzone we can use to avoid memory allocation
  // not limited by pow2 restriction!
  const uint32_t readsize_;
  BUFFER_UNIT* readzone_;

  pc_marker reader_thread_;
  pc_marker writer_thread_;

  // Mask functions

  /**
   * Mask a given value based on the length of the buffer.
   * 
   * Arguments:
   *  - input, the value which we are masking.
   * 
   * Returns:
   *  
   */ 
};