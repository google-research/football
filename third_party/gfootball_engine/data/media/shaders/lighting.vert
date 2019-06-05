#version 150

#pragma optimize(on)

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// eye-space
in vec4 position;

out mat4 modelViewMatrix;
out vec4 modelViewPosition;

void main(void) {
  modelViewMatrix = viewMatrix * modelMatrix;
  modelViewPosition = modelViewMatrix * position;
  gl_Position = projectionMatrix * modelViewPosition;
}
