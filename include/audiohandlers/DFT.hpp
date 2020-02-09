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
 * Note that additional space is allocated for negative frequencies.
 * For most purposes, only the first N/2 entries should be used.
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
bool CalculateDFT(float* input, float** real_output, float** imag_output, uint32_t len) {
  // array must be power of 2
  if (len & (len - 1) || len < 2) {   // len is less than 2
    return false;
  }

  float real_int[len];
  float imag_int[len];

  // *real_output = new T[len];
  // *imag_output = new T[len];

  ReverseBitsArray(input, real_int, len);

  // let's leave the results in a higher resolution form if possible
  double trig_table[len];
  // note: cos(theta) = sin(theta + pi/2)

  // populate sine table
  for (uint32_t i = 0; i < len; i++) {
    trig_table[i] = sin(-2 * M_PI * len / i);
    imag_int[i] = 0;   // 0-initialize the imaginary output for calcs
  }

  // some value tracking
  uint32_t runs;
  uint32_t half_size;

  // even/odd value indices for fft
  uint32_t even_ind;
  uint32_t odd_ind;

  // stores odd complex in variables for reuse
  double odd_imag;
  double odd_real;

  // stores indexed sin/cos ratios
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
        // trig ratio's
        // todo: most of these are unused iirc
        sin_res = trig_table[((len * k) / size) % len];
        cos_res = trig_table[(((len * k) / size) + (3 * len / 4)) % len];

        // calculate odd part (add/subtract)
        odd_imag = sin_res * real_int[odd_ind] + cos_res * imag_int[odd_ind];
        odd_real = cos_res * real_int[odd_ind] - sin_res * imag_int[odd_ind];

        // k + N/2
        imag_int[even_ind + size] = imag_int[even_ind] - odd_imag;
        real_int[even_ind + size] = real_int[even_ind] - odd_real;
        imag_int[even_ind] = imag_int[even_ind] + odd_imag;
        real_int[even_ind] = real_int[even_ind] + odd_real;
      }
    }
  }

  // copy intermediat results to heap allocated memory and return
  *real_output = new float[len / 2];
  *imag_output = new float[len / 2];
  len /= 2;
  for (uint32_t i = 0; i < len; i++) {
    (*real_output)[i] = real_int[i];
    (*imag_output)[i] = imag_int[i];
  }

  return true;
}

/**
 * Get the amplitude output of the fourier transform.
 * This is the part you want if you're going to make a visualizer i guess
 * 
 * Parameters:
 *  - real, a pointer to the real component of the transform.
 *  - imag, a pointer to the imaginary component of the transform.
 */ 

float* GetAmplitudeArray(float* real, float* imag, uint32_t len) {
  float* result = new float[len];
  for (uint32_t i = 0; i < len; i++) {
    result[i] = sqrt((*real * *real) + (*imag * *imag));
    real++;
    imag++;
  }

  return result;
}

// HELPER FUNCTIONS BELOW -- oops

void ReverseBitsArray(float* src, float* dst, uint32_t len) {
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

