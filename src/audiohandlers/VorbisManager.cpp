#include "audiohandlers/VorbisManager.hpp"

#include "vorbis/stb_vorbis.h"



VorbisManager::VorbisManager(char* filename) : audiofile_(nullptr), critical_buffer_capacity_(0),
                                               critical_buffer_(), buffer_list_() {
  int error_detect;

  audiofile_ = stb_vorbis_open_filename(filename, &error_detect, NULL);

  stb_vorbis_info info = stb_vorbis_get_info(audiofile_);
  sample_rate_ = info.sample_rate;
  channel_count_ = info.channels;

  // don't spin up the read thread, because we haven't created a critical thread yet.
}

std::shared_ptr<AudioBufferSPSC<float>> VorbisManager::CreateCriticalBuffer(int twopow) {
  if (std::weak_ptr<AudioBufferSPSC<float>>{}.owner_before(critical_buffer_) ||
      critical_buffer_.owner_before(std::weak_ptr<AudioBufferSPSC<float>>{})) {
        // trick from https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned  
        // buffer already exists if we're in here
        // check if it's expired
        if (critical_buffer_.expired()) {
          // old buffer is dead
          // ensure length field is valid

          auto shared_buffer = std::shared_ptr<AudioBufferSPSC<float>>(new AudioBufferSPSC<float>(twopow));
          critical_buffer_ = std::weak_ptr(shared_buffer);
          return shared_buffer;
        }

        // if its still around, bump it onto the list

        // check the list to see what the minimum buffer capacity is
        
  }
  AudioBufferSPSC<float>* buf = new AudioBufferSPSC<float>(twopow);
}

int VorbisManager::GetCriticalBufferMaxPower() {

  auto itr = buffer_list_.begin();
  while (itr != buffer_list_.end()) {
    if (itr->expired) {
      
    }
  }
}

