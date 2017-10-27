#version 410

in vec4 vVertex;
in vec2 vTexCoords;

smooth out vec2 vVaryingTexCoords;

void main(void)
{
    vVaryingTexCoords = vTexCoords;
    gl_Position = vVertex;
}
