#include <string>
#include <fstream>
#include <sstream>

#include <iostream>

#include "gl/GL.hpp"
#include "stbi.h"

bool GL::CreateProgram(const std::string& vert, const std::string& frag, GLuint* output) {
  
  std::ifstream vert_file(vert);
  
  if (vert_file.bad() || vert_file.fail()) {
    return false;
  }

  std::ifstream frag_file(frag);
  
  if (vert_file.bad() || vert_file.fail()) {
    return false;
  }

  std::stringstream vert_stream;
  std::stringstream frag_stream;

  // LoGL + https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
  // apparently this is as close to C I/O as you can get
  // and C I/O is the fastest one...!
  vert_stream << vert_file.rdbuf();
  frag_stream << frag_file.rdbuf();

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

  const std::string vert_contents = vert_stream.str();
  const std::string frag_contents = frag_stream.str();


  const char* vert_ptr = vert_contents.c_str();
  const char* frag_ptr = frag_contents.c_str();

  glShaderSource(vert_shader, 1, &vert_ptr, NULL);
  glShaderSource(frag_shader, 1, &frag_ptr, NULL);

  vert_file.close();
  frag_file.close();

  GLint success;

  glCompileShader(vert_shader);
  glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(vert_shader, 512, NULL, info);
    std::cout << "error while compiling vertex shader:\n" << info << std::endl;

    glDeleteShader(vert_shader);
    return false;
  }

  glCompileShader(frag_shader);
  glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(frag_shader, 512, NULL, info);
    std::cout << "error while compiling fragment shader: " << info << std::endl;

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return false;
  }


  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert_shader);
  glAttachShader(prog, frag_shader);
  glLinkProgram(prog);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  glGetProgramiv(prog, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(prog, 512, NULL, info);
    std::cout << "error while linking shaders:\n" << info << std::endl;
    glDeleteProgram(prog);
    return false;
  }

  *output = prog;
  return true;
}

GLuint GL::CreateVBOFromArray(void* data, GLuint size, GLint usage) {
  GLuint buf;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage);
  return buf;
}

GLuint GL::CreateEBOFromArray(unsigned int* data, GLuint size, GLint usage) {
  GLuint ind;
  glGenBuffers(1, &ind);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ind);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);

  return ind;
}

bool GL::CreateTextureFromFilename(const std::string& filename,
                                   GLint internal_format,
                                   GLint format,
                                   GLint datatype,
                                   GLuint* output)
{
  return CreateTextureFromFilename(filename, internal_format, format, datatype, GL_TEXTURE0, output);
}

bool GL::CreateTextureFromFilename(const std::string& filename,
                                   GLint internal_format,
                                   GLint format,
                                   GLint datatype,
                                   GLint texunit,
                                   GLuint* output)
{
  int width, height, nr_channels;
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nr_channels, 0);

  if (data == NULL) {
    return false;
  }

  glActiveTexture(texunit);

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, datatype, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return true;
}