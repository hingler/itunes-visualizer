

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

template <typename BUFFER_UNIT>
class AudioBufferSPSC {

 public:
  AudioBufferSPSC(int twopowcapacity, int readsize) : 
  
 private:
 

  const uint32_t buffer_capacity_;
  BUFFER_UNIT* buffer_;

  std::atomic_uint32_t shared_read_;
  std::atomic_uint32_t shared_write_;

  const uint32_t readsize_;

  BUFFER_UNIT* copy_storage_;
};