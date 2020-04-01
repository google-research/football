#version 150

in vec2 TexCoords;
out vec4 stdout;

uniform sampler2D ourTexture;

void main()
{
    stdout = texture(ourTexture, TexCoords);
}