#include "shaders/WaveShader.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "audiohandlers/DFT.hpp"

#include <iostream>

#include <vector>

WaveShader::WaveShader() {
  // create program

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenTextures(1, &uDftTex_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, uDftTex_);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 8192, 128, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // create tex buffer
  glGenBuffers(1, &aPos_);
  glBindBuffer(GL_ARRAY_BUFFER, aPos_);

  // define geometry

  std::vector<float> vertex_cache(3 * VERTEX_DEPTH * VERTEX_WIDTH, 0.0f);

  int index;

  for (int i = 0; i < VERTEX_DEPTH; i++) {
    for (int j = 0; j < VERTEX_WIDTH; j++) {
      index = (i * VERTEX_WIDTH + j) * 3;
      vertex_cache[index] = (SPACE_WIDTH * 2.0 / VERTEX_WIDTH) * j - SPACE_WIDTH;
      vertex_cache[index + 1] = 0.0f;
      vertex_cache[index + 2] = -(SPACE_DEPTH / VERTEX_DEPTH) * i;
    }
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_cache.size(), vertex_cache.data(), GL_STATIC_DRAW);

  // attrib layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  glBindAttribLocation(prog_, 0, "aPos");
  glEnableVertexAttribArray(0);

  glUseProgram(prog_);

  // get uniform locs
  texOffsetY_ = glGetUniformLocation(prog_, "texOffsetY");
  spaceWidth_ = glGetUniformLocation(prog_, "spaceWidth");
  spaceDepth_ = glGetUniformLocation(prog_, "spaceDepth");
  viewMatrix_ = glGetUniformLocation(prog_, "viewMatrix");
  dftHistory_ = glGetUniformLocation(prog_, "dftHistory");
}

void WaveShader::Render(GLFWwindow* window, float* sample_data, size_t length) {
  // sumn
  glm::mat4 persp = glm::perspective(75.0, (16.0 / 9.0), 0.1, 1000.0);
  // this is probably it so whatever

  uint32_t maxlen = 1;
  while (maxlen <= length) {
    maxlen <<= 1;
  }

  maxlen >>= 1;

  dft::CalculateDFT(sample_data, real_output, imag_output, maxlen);
  dft::GetAmplitudeArray(real_output, imag_output, ampl_output, maxlen);

  glUseProgram(prog_);
  glBindVertexArray(vao_);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, uDftTex_);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, y_offset_, maxlen, 1, GL_RED, GL_FLOAT, ampl_output);

}