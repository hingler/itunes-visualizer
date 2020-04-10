#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "gl/GL.hpp"

#include "stbi.h"

#include <iostream>

static float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};

static float tcoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

unsigned int indices[] = {
  0, 1, 2,
  2, 3, 0
};

void SizeChangeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
  if (!glfwInit()) {
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


  const int width = 1024;
  const int height = 768;
  GLFWwindow* window = glfwCreateWindow(width, height, "world of cringe", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "failed to load glad" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glViewport(0, 0, width, height);

  glfwSetFramebufferSizeCallback(window, SizeChangeCallback);

  GLuint prog;

  if (!GL::CreateProgram("resources/texturetest/samplevert.vert.glsl",
                         "resources/texturetest/samplefrag.frag.glsl", &prog)) 
    {
      std::cout << "failed to create program." << std::endl;
      glfwTerminate();
      return EXIT_FAILURE;
    }

  // RAII Wrappers for VAO/VBO/EBO would be a good idea
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo = GL::CreateVBOFromArray(reinterpret_cast<void*>(vertices), sizeof(vertices), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);
  
  GLuint vbo_tex = GL::CreateVBOFromArray(reinterpret_cast<void*>(tcoords), sizeof(tcoords), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, (void*)0);
  
  GLuint ebo = GL::CreateEBOFromArray(indices, sizeof(indices), GL_STATIC_DRAW);

  glBindAttribLocation(prog, 0, "aPos");
  glBindAttribLocation(prog, 1, "aTex");

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // texture!

  // load the content
  int iwidth;
  int iheight;
  int ichannels;

  stbi_set_flip_vertically_on_load(1);
  unsigned char* data = stbi_load("resources/monkey.png", &iwidth, &iheight, &ichannels, 0);

  if (!data) {
    std::cout << "Failed to load the monkey" << std::endl;
    stbi_image_free(data);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  GLuint tex;
  glGenTextures(1, &tex);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iwidth, iheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  GLuint textwo;
  glGenTextures(1, &textwo);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textwo);

  data = stbi_load("resources/untitled.png", &iwidth, &iheight, &ichannels, 0);

  if (!data) {
    std::cout << "Failed to load beet" << std::endl;
    stbi_image_free(data);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textwo);

  glUseProgram(prog);

  glUniform1i(glGetUniformLocation(prog, "tex"), 0);
  glUniform1i(glGetUniformLocation(prog, "textwo"), 1);
  glUniform1f(glGetUniformLocation(prog, "time"), glfwGetTime());
  // uniforms can only be bound if using a program

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glBindVertexArray(vao);

    glUniform1f(glGetUniformLocation(prog, "time"), glfwGetTime());

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }
}

void SizeChangeCallback(GLFWwindow* window, int width, int height) {
  std::cout << "new size: " << width << " x " << height << std::endl;
  glViewport(0, 0, width, height);
}