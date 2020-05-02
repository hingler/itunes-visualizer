#ifndef WAVE_SHADER_H_
#define WAVE_SHADER_H_

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "shaders/AudioShader.hpp"
#include "glm/vec2.hpp"

/**
 *  The WaveShader represents audio data via a set of vertices which extend back into the scene.
 *  You will enjoy these colors as they perform for you.
 */ 
class WaveShader : public AudioShader {
 public:
  WaveShader();
  void Render(GLFWwindow* window, float* sample_data, size_t length) override;
  const std::string* GetParameterNames() override;
  void SetParameter(const std::string& param_name, std::any value) override;
  ~WaveShader() override;

  const int VERTEX_DEPTH = 128;      // number of vertices along Z
  const int VERTEX_WIDTH = 512;    // number of vertices along X

  const int TEXTURE_WIDTH = 8192;
  const int TEXTURE_HEIGHT = 128;

  const float SPACE_WIDTH = 8.0f;   // width occupied by vertices in +/- x directions
  const float SPACE_DEPTH = 16.0f;  // depth occupied by vertices in -Z direction
 private:

  // regenerate framebuffer textures when screen size changes
  void CreateFramebufferTextures();
  // take a guess at the size
  glm::vec2 screensize_;

  // prog
  GLuint prog_;

  // screen display prog
  GLuint buffprog_;

  // tex id for dft data
  GLuint uDftTex_;

  // vao
  GLuint vao_;
  GLuint buffer_vao_;

  // vertex buffer
  GLuint aPos_;
  GLuint buffer_aPos_;

  // frame buffer + textures
  GLuint framebuffer_;
  GLuint fb_color_;
  GLuint fb_depth_stencil_;

  // other uniforms
  GLuint texOffsetY_;  // for reading buffer history
  GLuint spaceWidth_;  // z range
  GLuint spaceDepth_;  // x range
  GLuint viewMatrix_;  // view matrix
  GLuint dftHistory_;  // texture spot
  GLuint cameraPozz_;  // camera position

  GLuint image_;      // uniform location for image on fb renderer

  // moving forward: might be best to merge these into a struct in cases
  // where there are multiple shaders on a single render call

  // where we should start reading from
  int y_offset_;

  // WORK SPACE
  const static int BUFFER_SIZE = 8192;
  float real_output[BUFFER_SIZE];
  float imag_output[BUFFER_SIZE];
  float ampl_output[BUFFER_SIZE];
};

#endif  // WAVE_SHADER_H_