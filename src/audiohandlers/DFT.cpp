#define _USE_MATH_DEFINES
#include "audiohandlers/DFT.hpp"

#include <cinttypes>
#include <cmath>

#include <array>
#include <vector>

namespace dft {
bool CalculateDFT(float* input, float** real_output, float** imag_output, uint32_t len) {
  if (len & (len - 1) || len < 2) {
    return false;
  }

  std::vector<float> real_int(len);
  std::vector<float> imag_int(len);

  ReverseBitsArray(input, real_int, len);

  std::vector<double> sin_table(len / 2);
  std::vector<double> cos_table(len / 2);
  // note: cos(theta) = sin(theta + pi/2)

  for (uint32_t i = 0; i < (len / 2); i++) {
    // thanks up to https://github.com/dntj/jsfft
    // for helping me realize i had my trig ratios all janked
    sin_table.push_back(sin((-2 * M_PI * i) / len));
    cos_table.push_back(cos((-2 * M_PI * i) / len));
  }

  for (uint32_t i = 0; i < len; i++) {
    imag_int[i] = 0;
  }

  uint32_t even_ind;
  uint32_t odd_ind;

  double odd_imag;
  double odd_real;

  double sin_res;
  double cos_res;

  for (uint32_t size = 1; size < len; size <<= 1) {
    uint32_t runs = (len / size) / 2;
    // # of separate DFT ops
    for (uint32_t i = 0; i < runs; i++) {
      // single fft call with size (size)
      for (uint32_t k = 0; k < size; k++) {
        even_ind = i * size * 2 + k;
        odd_ind = even_ind + size;

        sin_res = sin_table[(k * len) / (2 * size)];
        cos_res = cos_table[(k * len) / (2 * size)];
        
        // calculate odd part (add/subtract)
        odd_imag = sin_res * real_int[odd_ind] + cos_res * imag_int[odd_ind];
        odd_real = cos_res * real_int[odd_ind] - sin_res * imag_int[odd_ind];

        imag_int[size + even_ind] = static_cast<float>(imag_int[even_ind] - odd_imag);
        real_int[size + even_ind] = static_cast<float>(real_int[even_ind] - odd_real);
        imag_int[even_ind] = static_cast<float>(imag_int[even_ind] + odd_imag);
        real_int[even_ind] = static_cast<float>(real_int[even_ind] + odd_real);
      }
    }
  }

  *real_output = new float[len];
  *imag_output = new float[len];

  for (uint32_t i = 0; i < len; i++) {
    (*real_output)[i] = real_int[i];
    (*imag_output)[i] = imag_int[i];
  }

  return true;
}

float* GetAmplitudeArray(float* real, float* imag, uint32_t len) {
  float* result = new float[len];
  for (uint32_t i = 0; i < len; i++) {
    result[i] = sqrt((*real * *real) + (*imag * *imag));
    real++;
    imag++;
  }

  return result;
}

void ReverseBitsArray(float* src, std::vector<float>& dst, uint32_t len) {
  uint8_t bit_width = 0;

  uint32_t len_copy = len - 1;
  while (len_copy > 0) {
    len_copy >>= 1;
    bit_width++;
  }

  uint32_t reversed_index;
  for (uint32_t i = 0; i < len; i++) {
    reversed_index = ReverseBits(i, bit_width);
    dst[i] = src[reversed_index];
  }
}

uint32_t ReverseBits(uint32_t input, uint8_t bit_width) {
  uint32_t result = 0;
  for (int i = 0; i < bit_width; i++) {
    result <<= 1;
    result += input & 1;
    input >>= 1;
  }

  return result;
}

}  // namespace dft;