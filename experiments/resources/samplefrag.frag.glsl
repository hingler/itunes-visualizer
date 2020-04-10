#version 410 core

out vec4 FragColor;
in vec4 vertexColor;

void main() {
  FragColor.rgb = pow(vertexColor.rgb, vec3(1.0 / 2.2));
  FragColor.a = 1.0;
}