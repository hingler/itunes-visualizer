
/**
 *  Abstract class which interacts with render thread to produce visuals.
 */ 

#include <any>
#include <string>

class AudioShader {
 public:
  // constructor
  AudioShader();

  /**
   *  Draws our shader to the currently active window.
   *  sample_data: pointer to raw samples
   *  length: number of elements
   */ 
  virtual void Render(float* sample_data, int length) = 0;

  /**
   *  Returns name of all configurable inputs to the shader.
   */ 
  virtual const std::string* GetParameterNames() = 0;

  /**
   *  Sets the value associated with a given parameter.
   */ 
  virtual void SetParameter(std::string& param_name, std::any value) = 0;

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
      *T = std::any_cast<T>(target);
    } catch (const std::bad_any_cast& e) {
      return false;
    }

    return true;
  }

};  // class AudioShader