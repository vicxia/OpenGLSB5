#version 410

out vec4 vFragColor;

uniform sampler2D colorMap;

smooth in vec2 vVaryingTexCoords;

void main(void)
{
    vFragColor = texture(colorMap, vVaryingTexCoords.st);
}

