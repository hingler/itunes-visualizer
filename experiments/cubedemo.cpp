// quickly putting a cube on the screen to get myself back up to speed
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>

// called on a window size change
void SizeChangeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
  
  // all of this is from LearnOpenGL.com -- again, just getting myself refreshed
  if (!glfwInit()) {
    return 1;
  }

  // glad loader created for OGL 4.1
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  // use core profile vs. compat profile (immediate vs. retained iirc)
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // GLFW is our context creator, so we use it to create a window

  // w/h/name, monitor (NULL = windowed mode), share (a window to share resources with)
  GLFWwindow* window = glfwCreateWindow(1024, 768, "creatine", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    return 1;
  }

  // this context is the one we are going to work with
  glfwMakeContextCurrent(window);

  // now we want to load GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "something's up with glad" << std::endl;
    glfwTerminate();
    return 1;
  }

  // we now have access to GL functions

  // (no constants, this would be ideal)
  // tells OGL how to map gl coords to screen coords
  glViewport(0, 0, 1024, 768);

  // modify viewport if framebuffer size (window size) changes
  glfwSetFramebufferSizeCallback(window, SizeChangeCallback);

  // while the window is open
  while (!glfwWindowShouldClose(window)) {
    // double buffered -- swap buffers so that we can start working
    glfwSwapBuffers(window);
    // checks for input and calls callbacks assc'd with them
    glfwPollEvents();
  }

  // here: the window has been closed
  
  std::cout << "closing..." << std::endl;
  glfwTerminate();
  return 0;
}

void SizeChangeCallback(GLFWwindow* window, int width, int height) {
  std::cout << "new size: " << width << " x " << height << std::endl;
  glViewport(0, 0, width, height);
}