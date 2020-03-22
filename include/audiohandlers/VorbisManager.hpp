
// fuck!my hi.freq tonal alarms are going off the walls...

// provide interface for communicating back+forth with stb vorbis 
#ifndef VORBIS_MANAGER_H_
#define VORBIS_MANAGER_H_
#include "audiohandlers/AudioBufferSPSC.hpp"
#include "vorbis/stb_vorbis.h"
#include <memory>
#include <list>
#include <mutex>
#include <thread>

// a lot of this stuff is provided by the lib already
// but the aim is to just make some c calls into cpp calls

class VorbisManager {
  // floats only!

  // allows the user to request additional buffers as needed
  // loads content into these buffers as required
  // synchronizes buffers as needed

  // should be constructed with a file object generated from a vorbis stream
 public:
  /**
   * Create a vorbis manager which reads from a given OGG file.
   * Sets up a critical buffer and prepares itself to begin reading into it.
   * 
   * Args:
   *  - filename, the path associated with the filename you are opening
   *  - twopow, the log_2 of the size of the desired buffer. Should keep as small as possible
   */  
  VorbisManager(char* filename, int twopow);
  
  /**
   *  Constructs a new audio buffer on the heap which will receive
   *  input from the manager, returning a shared pointer to it.
   * 
   *  All buffers created must be larger than the critical buffer.
   *  If client attempts to create a buffer which is smaller than the critical buffer,
   *  returns nullptr.
   * 
   *  Must be freed by the user.
   */ 
  std::shared_ptr<AudioBufferSPSC<float>> CreateBufferInstance(int twopow);

  /**
   *  Starts the write thread.
   */ 
  void StartWriteThread();
  /**
   *  Stops the write thread.
   */ 
  void StopWriteThread();

  VorbisManager(VorbisManager&) = delete;
  void operator=(const VorbisManager& m) = delete;
  ~VorbisManager();

 private:
  std::list<std::weak_ptr<AudioBufferSPSC<float>>> buffer_list_;
  std::shared_ptr<AudioBufferSPSC<float>> critical_buffer_;
  uint32_t critical_buffer_capacity_;
  stb_vorbis* audiofile_;

  // some const fields
  unsigned int sample_rate_;
  int channel_count_;

  std::thread write_thread_;

  float* channel_buffers_;

  // lock for buffer list
  std::mutex buffer_list_lock;

  // troves thru the buffer vector and erases any expired entries
  void ClearFreedBuffers();

  /**
   *  The function which will populate our buffers when called
   *  WriteThreadFn will perform some check to see if we can call this
   */ 
  void PopulateBuffers(uint32_t write_size);

  /**
   *  Function called by our write thread
   *  For now: all buffers are populated by the same thread
   *  But if performance is an issue that can be rearranged (it probably wont be)
   *  Idea: ensure 64 byte size on individual audio queues?
   */ 
  void WriteThreadFn(void* vorbis_inst);
};

// The AudioBuffer provides a convenient, low-footprint read/write structure
// which minimizes read footprint

// however, we've now run into the issue of synchronizing activity
// between the fft threads

// we can control the rate at which the buffer is read, but we also need to prevent them
// from desynchronizing

// how would i do it?

// audiobuffer stores decoded data from our file on disk
// if the buffers are completely full, there will be some event in which one buffer
// can be read to but not the other.

// why not make the playback buffer smaller than the other buffers?
// then, whenever we can successfully write to the playback buffer,
// we also write to the remaining buffers.

// the playback buffer is the most crucial, so we'll fill by it.
// since our other buffers are bound to be read at roughly the same rate,
// we'll empty them roughly accordingly, but add some buffer space in case it gets ahead or behind.

#endif  // VORBIS_MANAGER_H_