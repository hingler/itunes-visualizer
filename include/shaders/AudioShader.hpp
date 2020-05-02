#ifndef AUDIO_SHADER_H_
#define AUDIO_SHADER_H_


/**
 *  Abstract class which interacts with render thread to produce visuals.
 */ 

#include <any>
#include <string>

#include "GLFW/glfw3.h"

class AudioShader {
 public:

  /**
   *  Draws our shader to the currently active window.
   *  sample_data: pointer to raw samples
   *  length: number of elements
   */ 
  virtual void Render(GLFWwindow* window, float* sample_data, size_t length) = 0;

  /**
   *  Returns name of all configurable inputs to the shader.
   */ 
  virtual const std::string* GetParameterNames() = 0;

  /**
   *  Sets the value associated with a given parameter.
   */ 
  virtual void SetParameter(const std::string& param_name, std::any value) = 0;

  virtual ~AudioShader() { }

 protected:
  // full screen rect
  const float boxCoords[8] = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

 private:
  
  /**
   *  Helper function which encompasses our std::any cast
   * 
   *  Returns true if cast is successful, and places result in output.
   *  Returns false otherwise.
   */ 
  template <typename T>
  bool AttemptCast(std::any& target, T* output) {
    try {
      *output = std::any_cast<T>(target);
    } catch (const std::bad_any_cast& e) {
      return false;
    }

    return true;
  }

};  // class AudioShader

#endif  // AUDIO_SHADER_H_