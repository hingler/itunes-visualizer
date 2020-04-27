#include "shaders/WaveShader.hpp"
#include <iostream>

#include <vector>

WaveShader::WaveShader() {
  // create program

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
}

void WaveShader::Render(GLFWwindow* window, float* sample_data, size_t length) {
  
}