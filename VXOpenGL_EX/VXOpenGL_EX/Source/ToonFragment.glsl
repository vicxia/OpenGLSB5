#version 410

out vec4 vFragColor;

smooth in float textureCoordinate;

uniform sampler1D colorTable;

void main(void)
{
    vFragColor = texture(colorTable, textureCoordinate);
}
