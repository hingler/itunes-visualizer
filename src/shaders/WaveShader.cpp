#include "audiohandlers/DFT.hpp"
#include "shaders/WaveShader.hpp"
#include "gl/GL.hpp"

#include "glm/mat4x4.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"


#include <iostream>

#include <vector>

WaveShader::WaveShader() : y_offset_(0), screensize_(512, 256) {
  // create program

  if (!GL::CreateProgram("resources/waveshader/waveshader.vert.glsl",
                         "resources/waveshader/waveshader.frag.glsl",
                         &prog_)) {
    std::cout << "failed to load program" << std::endl;
    std::cin.ignore();
    exit(EXIT_FAILURE);
  }

  if (!GL::CreateProgram("resources/waveshader/screen.vert.glsl",
                         "resources/waveshader/gaussianblur.frag.glsl",
                         &buffprog_)) {
    std::cout << "failed to create screen prog" << std::endl;
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

  // create framebuffer
  glGenFramebuffers(1, &framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  glGenTextures(1, &fb_color_);
  glGenTextures(1, &fb_depth_stencil_);

  // take a random guess at the window width / size
  // record that guess in a private param
  CreateFramebufferTextures();

  // create tex buffer
  glGenBuffers(1, &aPos_);
  glBindBuffer(GL_ARRAY_BUFFER, aPos_);


  // note: if color and stencil are next to each other we can probably gen them in one sweep
  // create color texture
  // maybe i can use the depth buffer at some point so i dont want to relegate it to a render buffer yet

  // define geometry

  std::vector<float> vertex_cache(3 * VERTEX_DEPTH * VERTEX_WIDTH, 0.0f);

  int index;

  for (int i = 0; i < VERTEX_DEPTH; i++) {
    for (int j = 0; j < VERTEX_WIDTH; j++) {
      index = (i * VERTEX_WIDTH + j) * 3;
      vertex_cache[index] = static_cast<float>((SPACE_WIDTH * 2.0 / VERTEX_WIDTH) * j - SPACE_WIDTH);
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

  // create VAO for the screen thing
  glGenVertexArrays(1, &buffer_vao_);
  glBindVertexArray(buffer_vao_);

  glGenBuffers(1, &buffer_aPos_);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_aPos_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(boxCoords), boxCoords, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));
  glBindAttribLocation(buffer_aPos_, 0, "aPos");
  glEnableVertexAttribArray(0);

  glUseProgram(buffprog_);

  // uniforms for blur prog
  image_ = glGetUniformLocation(buffprog_, "image");
  width_ = glGetUniformLocation(buffprog_, "width");
  weights_ = glGetUniformLocation(buffprog_, "weights");
  screencoord_ = glGetUniformLocation(buffprog_, "screencoord");
  horizontal_ = glGetUniformLocation(buffprog_, "horizontal");
}

void WaveShader::Render(GLFWwindow* window, float* sample_data, size_t length) {
  // glEnable(GL_LINE_SMOOTH);
  // sumn
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  int width;
  int height;
  glfwGetFramebufferSize(window, &width, &height);
  if (width != screensize_.x || height != screensize_.y) {
    // recreate frame buffers
    screensize_.x = width;
    screensize_.y = height;
    CreateFramebufferTextures();
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    // mostly for diagnostic purposes -- not sure if this will be triggered
    std::cout << "framebuffer is incomplete" << std::endl;
  }

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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(prog_);
  glBindVertexArray(vao_);

  glPointSize(4.0f);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, uDftTex_);

  // output the dft data to the texture
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, (y_offset_ % TEXTURE_HEIGHT), maxlen, 1, GL_RED, GL_FLOAT, ampl_output);
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

  // final draw
  glBindFramebuffer(GL_FRAMEBUFFER, NULL);

  // clearing this caused the image to appear
  // not sure why though lmao

  glUseProgram(buffprog_);
  glBindVertexArray(buffer_vao_);

  glActiveTexture(GL_TEXTURE0);
  // bind the buffer
  glBindTexture(GL_TEXTURE_2D, fb_color_);
  
  glUniform1i(image_, 0);
  glUniform2f(screencoord_, screensize_.x, screensize_.y);
  glUniform1i(horizontal_, 1);
  glUniform1f(width_, BLUR_WIDTH);

  std::vector<float> weights;
  for (int i = 0; i < BLUR_WIDTH; i++) {
    // i ^ 2 / 2 * sigma ^ 2
    // width is assumed to be 3 * sigma
    weights.push_back(exp(-(i * i) / (2 * (BLUR_WIDTH * BLUR_WIDTH / 9.0))));
  }

  glUniform1fv(weights_, BLUR_WIDTH, weights.data());

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

const std::string* WaveShader::GetParameterNames() {
  return nullptr;
}

void WaveShader::SetParameter(const std::string& param_name, std::any value) {

}

void WaveShader::CreateFramebufferTextures() {
  // note: no longer necessary to delete + recreate texture on resize
  // you can just make another call to texImage2D

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  
  glBindTexture(GL_TEXTURE_2D, fb_color_);
  std::cout << static_cast<GLsizei>(screensize_.x) << std::endl;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(screensize_.x), static_cast<GLsizei>(screensize_.y), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_color_, 0);

  // prepare depth + stencil
  glBindTexture(GL_TEXTURE_2D, fb_depth_stencil_);
  // internal format is GL_UNSIGNED_INT_24_8
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(screensize_.x), static_cast<GLsizei>(screensize_.y), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
  // note: format is GL_DEPTH_STENCIL_ATTACHMENT
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fb_depth_stencil_, 0);
}

WaveShader::~WaveShader() {
  glDeleteBuffers(1, &aPos_);
  glDeleteFramebuffers(1, &framebuffer_);
  glDeleteVertexArrays(1, &vao_);
}