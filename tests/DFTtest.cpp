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

  // todo: figure out a proper way to test this

  // float test_two[128];
  // for (int i = 0; i < 128; i++) {
  //   // we should expect four oscillations here
  //   test_two[i] = sin((M_PI / 16) * i);
  // }

  // delete[] real_output;
  // delete[] imag_output;

  // dft::CalculateDFT(test_two, &real_output, &imag_output, 128);
  // for (int i = 0; i < 128; i++) {
  //   std::cout << real_output[i] << " + " << imag_output[i] << "i" << std::endl;
  // }

  // std::cout << std::endl;
}