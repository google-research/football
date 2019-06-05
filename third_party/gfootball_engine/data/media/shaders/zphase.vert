#version 150

#pragma optimize(on)

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec4 position;

void main(void) {
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;
}
