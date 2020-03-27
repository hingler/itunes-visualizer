#include "audiohandlers/VorbisManager.hpp"

VorbisManager* VorbisManager::GetVorbisManager(int twopow, char* filename) {
  if (twopow <= 1) {
    // unreasonable
    return nullptr;
  }
  int err;
  stb_vorbis* file = stb_vorbis_open_filename(filename, &err, NULL);
  if (err != VORBIS__no_error) {
    // error occurred
    return nullptr;
  }

  // opened valid -- call constructor
  return new VorbisManager(twopow, file);
}