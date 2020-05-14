#include <audioreaders/VorbisReader.hpp>



VorbisReader::VorbisReader(stb_vorbis* file) {
  file_ = file;
}