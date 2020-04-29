#version 410 core

in float intensity;

out vec4 fragColor;

void main() {
  fragColor = vec4(vec3(intensity), 1.0f);
}