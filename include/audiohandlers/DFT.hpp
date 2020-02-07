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

template <typename T>
bool CalculateDFT(T* input, T* realOutput, T* imagOutput, int32_t len) {
  // array must be power of 2
  if (len & (len - 1) // len is not power of 2
      || len < 2) {   // len is less than 2
    return false;
  }
  // do we want to assume ownership of the input?
  // nah

  // uh
  T* input_copy = new T[];
  ReverseBitsArray(input, input_copy, len);

  // handle leaps
  for (uint32_t jump = 1; jump < len; jump *= 2) {
    
    // perform fft from bottom up
  }
}

template <typename T>
void ReverseBitsArray(T* src, T* dest, uint32_t len) {
  // again, len should be power of 2
  len /= 2;

  T temp;   // holds temp value pre-swap
  uint32_t reversedIndex;
  for (uint32_t i = 0; i < len; i++) {
    reversedIndex = ReverseBits(i);
    temp = arr[i];
    dest[i] = arr[reversedIndex];
    dest[reversedIndex] = temp;
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

