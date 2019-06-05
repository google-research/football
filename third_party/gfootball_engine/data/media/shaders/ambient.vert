#version 150

#pragma optimize(on)

// for the fullscreen quad
uniform mat4 orthoViewMatrix;
uniform mat4 orthoProjectionMatrix;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// eye-space
in vec4 position;

void main(void) {
  gl_Position = orthoProjectionMatrix * orthoViewMatrix * position;
}
