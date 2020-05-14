#include <audioreaders/AudioReader.hpp>
#include <stb_vorbis.h>

class VorbisReader : public AudioReader {
 public:
  bool GetSamplesInterleaved(int count, float* output) override;
  int GetSampleRate() override;
  int GetChannelCount() override;
  void Seek(int sample) override;
  static VorbisReader GetVorbisReader(std::string file);

 private:
  VorbisReader(stb_vorbis* file);
  stb_vorbis* file_;
};