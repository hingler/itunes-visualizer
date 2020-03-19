#include "audiohandlers/VorbisManager.hpp"

#include "vorbis/stb_vorbis.h"



VorbisManager::VorbisManager(char* filename, int twopow) : audiofile_(nullptr), critical_buffer_capacity_(0),
                                                           buffer_list_() {
  int error_detect;

  audiofile_ = stb_vorbis_open_filename(filename, &error_detect, NULL);

  stb_vorbis_info info = stb_vorbis_get_info(audiofile_);
  sample_rate_ = info.sample_rate;
  channel_count_ = info.channels;

  // initialize the critical buffer with given size
}

