#version 150

in vec4 vertex;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
   TexCoords = vertex.xy; // texture's x and y coordinates are the same as vertex coordinates
   gl_Position = projection * model * vertex;
}
