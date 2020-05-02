#version 410 core

uniform sampler2D image;

in vec2 texcoord;
out vec4 fragColor;

const float SIGMA = 4.0f;

// todo: make blur independent of screen size by accepting xy res as a uniform
const vec2 textureStep = vec2(1.0/1920, 0.0/256);

void main() {
  // blur and add for glow
  // horiz blur
  float acc = 0.0f;
  int limit = int(3 * SIGMA);

  float buf = 0.0f;

  vec4 result = vec4(0);

  // horiz step
  for (int i = -limit; i <= limit; i++) {
    buf = exp(-(i * i) / (2 * SIGMA * SIGMA));
    acc += buf;
    result += texture(image, (texcoord + (textureStep * i))) * buf;
  }

  result /= acc;

  // vec4 texsample = texture(image, texcoord);
  fragColor = result * 1.66 + texture(image, texcoord);
}