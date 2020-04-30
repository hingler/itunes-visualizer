#version 410 core

uniform float spaceDepth;

in float intensity;
in float z_dist;

out vec4 fragColor;

void main() {
  fragColor = vec4(vec3(((spaceDepth - z_dist) / spaceDepth)), 1.0f);
}