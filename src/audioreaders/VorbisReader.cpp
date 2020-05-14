#include <audioreaders/VorbisReader.hpp>

VorbisReader::VorbisReader(stb_vorbis* file) {
  file_ = file;
  info_ = stb_vorbis_get_info(file_);
}

VorbisReader* VorbisReader::GetVorbisReader(std::string file) {
  if (file.empty()) {
    return nullptr;
  }

  int err;
  stb_vorbis* vorbis_file = stb_vorbis_open_filename(file.c_str(), &err, NULL);
  if (vorbis_file == NULL) {
    return nullptr;
  }

  return new VorbisReader(vorbis_file);
}

int VorbisReader::GetSamplesInterleaved(int count, float* output) {
  std::lock_guard lock(read_lock_);
  return stb_vorbis_get_samples_float_interleaved(file_, info_.channels, output, count);
}

int VorbisReader::GetSampleRate() {
  return info_.sample_rate;
}

int VorbisReader::GetChannelCount() {
  return info_.channels;
}

void VorbisReader::Seek(int sample) {
  std::lock_guard lock(read_lock_);
  stb_vorbis_seek(file_, sample);
}

VorbisReader::~VorbisReader() {
  stb_vorbis_close(file_);
}