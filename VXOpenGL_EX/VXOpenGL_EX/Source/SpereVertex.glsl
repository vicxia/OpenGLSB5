#version 410

in vec4 vVertex;
in vec2 vTexCoord;

uniform mat4 mvpMatrix;

smooth out vec2 vVaryingTexCoord;

void main(void)
{
    vVaryingTexCoord = vTexCoord;
    gl_Position = mvpMatrix * vVertex;
}
