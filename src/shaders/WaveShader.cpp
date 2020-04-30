#include "audiohandlers/DFT.hpp"
#include "shaders/WaveShader.hpp"
#include "gl/GL.hpp"

#include "glm/mat4x4.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"


#include <iostream>

#include <vector>

WaveShader::WaveShader() : y_offset_(0) {
  // create program

  if (!GL::CreateProgram("resources/waveshader/waveshader.vert.glsl",
                         "resources/waveshader/waveshader.frag.glsl",
                         &prog_)) {
    std::cout << "failed to load program" << std::endl;
    std::cin.ignore();
    exit(EXIT_FAILURE);
  }

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenTextures(1, &uDftTex_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, uDftTex_);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
  cameraPozz_ = glGetUniformLocation(prog_, "camera");
}

void WaveShader::Render(GLFWwindow* window, float* sample_data, size_t length) {
  // sumn
  glm::mat4 persp(1.0f);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glm::vec3 camera(0, 2, 4);
  
  
  persp = glm::perspective(glm::radians(45.0), (16.0 / 9.0), 0.1, 1000.0);
  persp = glm::translate(persp, -camera);
  // this is probably it so whateverns

  uint32_t maxlen = 1;
  while (maxlen <= length) {
    maxlen <<= 1;
  }

  maxlen >>= 1;

  dft::CalculateDFT(sample_data, real_output, imag_output, maxlen);
  dft::GetAmplitudeArray(real_output, imag_output, ampl_output, maxlen, true);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(prog_);
  glBindVertexArray(vao_);

  glPointSize(4.0f);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, uDftTex_);

  // output the dft data to the texture
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, (y_offset_ % TEXTURE_HEIGHT), maxlen, 1, GL_RED, GL_FLOAT, ampl_output);
  std::cout << "updating row " << (y_offset_ % TEXTURE_HEIGHT) << std::endl;
  y_offset_ = (y_offset_ + 1) % TEXTURE_HEIGHT;

  glUniform1f(texOffsetY_, static_cast<float>(y_offset_) / TEXTURE_HEIGHT);
  glUniform1f(spaceWidth_, SPACE_WIDTH);
  glUniform1f(spaceDepth_, SPACE_DEPTH);
  glUniformMatrix4fv(viewMatrix_, 1, GL_FALSE, glm::value_ptr(persp));
  glUniform1i(dftHistory_, 1);
  glUniform3fv(cameraPozz_, 1, glm::value_ptr(camera));

  for (int i = 0; i < VERTEX_DEPTH; i++) {
    glDrawArrays(GL_LINE_STRIP, VERTEX_WIDTH * i, VERTEX_WIDTH);
  }
}

const std::string* WaveShader::GetParameterNames() {
  return nullptr;
}

void WaveShader::SetParameter(const std::string& param_name, std::any value) {

}