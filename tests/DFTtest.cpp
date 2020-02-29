#include "gtest/gtest.h"
#include "audiohandlers/DFT.hpp"
#include <cmath>

TEST(DFTTests, EnsureRuns) {
  float test_values[4] = {1.0, -1.0, 1.0, -1.0};
  float* real_output;
  float* imag_output;

  float expected_real[4] = {0, 0, 4, 0};
  float expected_imag[4] = {0, 0, 0, 0};

  dft::CalculateDFT(test_values, &real_output, &imag_output, 4);

  for (int i = 0; i < 4; i++) {
    ASSERT_NEAR(expected_real[i], real_output[i], 0.0001);
    ASSERT_NEAR(expected_imag[i], imag_output[i], 0.0001);
  }
}