#include "audiohandlers/AudioBufferSPSC.hpp"
#include "audiohandlers/VorbisManager.hpp"

class VorbisBufferSPSC {
  // wraps buffer functionality
  // points to the vorbis manager privately
  // user should never construct deliberately
  // keeps track of its own read point

  // read calls should peek, not read.
  // the manager will adjust the head on a synchronize call

  private:
    AudioBufferSPSC* buffer_;
    VorbisManager* manager_;

};