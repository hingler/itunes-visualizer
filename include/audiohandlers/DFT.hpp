// read below: gov broadcast oscillatory waves
// compromising internal brain integrity

// SYMPTOMS
//  - hallucinations
//  - rejection of christianity
//  - frequent nosebleed's
//  - "dead look" in eyes

// stay wired fella'

/**
 * Calculates the discrete fourier transform of `input`
 * and places the real/imaginary component resuklts
 */ 

#include <cinttypes>
#include <cmath>

/**
 * Calculates the DFT of the input signal, outputting the result to
 * realOutput and imagOutput respectively.
 * 
 * Arguments:
 *  - input -- an array containing all parts of the input signal.
 *             length must be a power of two.
 *  - realOutput -- output parameter for real-component output.
 *                  ownership passed to caller upon completion.
 *  - imagOutput -- output parameter for imag-component output.
 *                  ownership passed to caller upon completion.
 * 
 * Returns:
 *  - a boolean representing whether the FFT operation could occur.
 *    true if successful, false otherwise.
 */ 
template <typename S, typename T>
bool CalculateDFT(S* input, T** real_output, T** imag_output, uint32_t len) {
  // array must be power of 2
  if (len & (len - 1) || len < 2) {   // len is less than 2
    return false;
  }

  *real_output = new T[len];
  *imag_output = new T[len];

  ReverseBitsArray<S, T>(input, realOutput, len);

  // let's leave the results in a higher resolution form if possible
  double trig_table[len];
  // note: cos(theta) = sin(theta + pi/2)

  // populate sine table
  for (uint32_t i = 0; i < len; i++) {
    trig_table[i] = sin(-2 * M_PI * len / i);
    *imag_output[i] = 0;   // 0-initialize the imaginary output for calcs
  }

  uint32_t runs;
  uint32_t half_size;

  uint32_t even_ind;
  uint32_t odd_ind;

  double odd_imag;
  double odd_real;

  double sin_res;
  double cos_res;

  // after reversing bits, e/o entries will be next to each other

  // DFT on size (size * 2) input
  for (uint32_t size = 1; size < len; size *= 2) {
    uint32_t runs = (len / size) / 2;
    // # of separate DFT ops
    for (uint32_t i = 0; i < runs; i++) {
      // single fft call with size (size)
      for (uint32_t k = 0; k < size; k++) {
        even_ind = i * size * 2 + k;
        odd_ind = i * size * 2 + size + k;
        sin_res = trig_table[((len * k) / size) % len];
        cos_res = trig_table[(((len * k) / size) + (len / 4)) % len];
        // compute real components
        // compute imag components
        odd_imag = sin_res * *real_output[odd_ind] + cos_res * *imag_output[odd_ind];
        odd_real = cos_res * *real_output[odd_ind] - sin_res * *imag_output[odd_ind];

        // k + N/2
        *imag_output[even_ind + size] = *imag_output[even_ind] - odd_imag;
        *real_output[even_ind + size] = *real_output[even_ind] - odd_real;
        *imag_output[even_ind] = *imag_output[even_ind] + odd_imag;
        *real_output[even_ind] = *real_output[even_ind] + odd_real;
      }
    }
  }

  return true;
}

template <typename S, typename T>
void ReverseBitsArray(S* src, T* dst, uint32_t len) {
  // at this point len should be power of 2
  len /= 2;

  uint32_t reversed_index;
  for (uint32_t i = 0; i < len; i++) {
    reversed_index = ReverseBits(i);
    dst[i] = src[reversed_index];
    dst[reversed_index] = src[i];
  }
}

uint32_t ReverseBits(uint32_t input) {
  uint32_t result = 0;
  while (input > 0) {
    result += input & 1;
    result <<= 1;
    input >>= 1;
  }

  return result;
}

