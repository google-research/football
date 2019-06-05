#version 150

/* matrix spaces primer :

  http://stackoverflow.com/questions/8207420/desktop-glsl-without-ftransform

    A model matrix transforms an object from object coordinates to world coordinates.
    A view matrix transforms the world coordinates to eye coordinates.
    A projection matrix converts eye coordinates to clip coordinates.

  Based on standard naming conventions, the mvpMatrix is projection * view * model, in that order.
  There is no other matrices that you need to multiply by.

  Projection is your projection matrix (either ortho or perspective),
  view is the camera transform matrix (NOT the modelview), and
  model is the position, scale, and rotation of your object.
*/

#pragma optimize(on)

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec4 position;
in vec3 normal;
in vec3 texcoord;
in vec3 tangent;
in vec3 bitangent;

// view space
out vec4 frag_position;
out vec3 frag_normal;
out vec3 frag_texcoord;
out vec3 frag_tangent;
out vec3 frag_bitangent;

void main(void) {
  mat4 modelViewMatrix = viewMatrix * modelMatrix;
  frag_position = modelViewMatrix * position;

  mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	frag_normal = normalMatrix * normal;
	frag_tangent = normalMatrix * tangent;
	frag_bitangent = normalMatrix * bitangent;
  frag_texcoord.st = texcoord.st;

  gl_Position = projectionMatrix * frag_position;
}
