
// fuck!my hi.freq tonal alarms are going off the walls...

// provide interface for communicating back+forth with stb vorbis 

#include "vorbis/stb_vorbis.h"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include <memory>

// a lot of this stuff is provided by the lib already
// but the aim is to just make some c calls into cpp calls

class VorbisManager {
  // floats only!

  // allows the user to request additional buffers as needed
  // loads content into these buffers as required
  // synchronizes buffers as needed

  // should be constructed with a file object generated from a vorbis stream
 public:
  VorbisManager(char* filename);
  /**
   *  Constructs a new audio buffer on the heap which will receive
   *  input from the manager, returning a shared pointer to it.
   * 
   *  Must be freed by the user.
   */ 
  shared_ptr<AudioBufferSPSC> GetBufferInstance(int twopow);
  ~VorbisManager();
}

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