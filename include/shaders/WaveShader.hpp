#ifndef MULTI_COLOR_SHADER_H_
#define MULTI_COLOR_SHADER_H_

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "shaders/AudioShader.hpp"

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

  const int VERTEX_DEPTH = 64;      // number of vertices along Z
  const int VERTEX_WIDTH = 1024;    // number of vertices along X

  const float SPACE_WIDTH = 8.0f;   // width occupied by vertices in +/- x directions
  const float SPACE_DEPTH = 16.0f;  // depth occupied by vertices in -Z direction
 private:

  // prog
  GLuint prog_;

  // tex id for dft data
  GLuint uDftTex_;

  // vao
  GLuint vao_;

  // vertex buffer
  GLuint aPos_;
};

#endif  // MULTI_COLOR_SHADER_H_