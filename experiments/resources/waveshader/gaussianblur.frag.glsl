#version 410 core

uniform sampler2D image;
uniform vec2 screencoord;
uniform bool horizontal;

in vec2 texcoord;
out vec4 fragColor;

// multiple color attachments
// mean outputting to multiple textures

// useful for rendering both the image and the bright pass in one swoop

uniform float width;        // width of blur (number of samples taken in each direction)
uniform float weights[48];  // weights -- must contain at least (3 * sigma) values.

// todo: make blur independent of screen size by accepting xy res as a uniform

void main() {
  // blur and add for glow
  // horiz blur

  vec2 texturestep = vec2((horizontal ? 1.0 : 0.0) / screencoord.x, (horizontal ? 0.0 : 1.0) / screencoord.y);

  float acc = 0.0f;

  float buf = 0.0f;

  vec4 result = vec4(0);

  // horiz step
  for (int i = int(-width); i <= width; i++) {
    buf = weights[abs(i)];
    acc += buf;
    result += texture(image, (texcoord + (texturestep * i))) * buf;
  }

  result /= acc;

  // vec4 texsample = texture(image, texcoord);
  fragColor = result;
}