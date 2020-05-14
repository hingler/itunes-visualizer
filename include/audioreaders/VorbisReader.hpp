#include <audioreaders/AudioReader.hpp>
#include <stb_vorbis.h>

#include <mutex>

class VorbisReader : public AudioReader {
 public:
  int GetSamplesInterleaved(int count, float* output) override;
  int GetSampleRate() override;
  int GetChannelCount() override;
  void Seek(int sample) override;
  static VorbisReader* GetVorbisReader(std::string file);

  void operator=(const VorbisReader& rhs) = delete;
  VorbisReader(const VorbisReader& other) = delete;
  ~VorbisReader();


 private:
  VorbisReader(stb_vorbis* file);
  stb_vorbis* file_;
  stb_vorbis_info info_;
  std::mutex read_lock_;
};