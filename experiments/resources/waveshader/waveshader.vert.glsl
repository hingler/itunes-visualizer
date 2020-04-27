#version 410 core

uniform float texOffsetY;
uniform sampler2D dftHistory;
uniform float texWidth;
uniform float texHeight;

uniform float spaceWidth;
uniform float spaceDepth;

uniform mat4 viewMatrix;

in vec4 vectorPos;

out float intensity;

const float INTENSITY_MULTIPLIER = 4.0f;

void main() {
  float xSample = (vectorPos.x / (2 * spaceWidth)) + 1;
  float ySample = fract(texOffsetY + (vectorPos.z / spaceDepth));
  intensity = texture(dftHistory, vec2(xSample, ySample)); // this should be fine

  gl_Position = vec4(vectorPos.x, intensity * INTENSITY_MULTIPLIER, vectorPos.zw);

}

