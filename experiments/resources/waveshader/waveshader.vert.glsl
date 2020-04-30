#version 410 core

uniform float texOffsetY;
uniform sampler2D dftHistory;

uniform float spaceWidth;
uniform float spaceDepth;

uniform vec3 camera;

uniform mat4 viewMatrix;

in vec4 vectorPos;

out float intensity;
out float z_dist;

const float INTENSITY_MULTIPLIER = 4.0f;

void main() {
  float xSample = 0.125 - (abs(vectorPos.x) / spaceWidth) / 8;
  float ySample = fract(texOffsetY + (vectorPos.z / spaceDepth));
  intensity = cos(xSample * 1.5708 * 32) * pow(texture(dftHistory, vec2(xSample, ySample)).r, 0.5); // this should be fine

  z_dist = length(camera - vectorPos.xyz);
  // i think this is right
  gl_Position = viewMatrix * vec4(vectorPos.x, intensity * INTENSITY_MULTIPLIER, vectorPos.zw);

}

