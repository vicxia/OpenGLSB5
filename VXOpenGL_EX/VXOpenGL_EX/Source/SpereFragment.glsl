#version 410

out vec4 vFragColor;

uniform sampler2DRect rectangleImage;

smooth in vec2 vVaryingTexCoord;

void main(void)
{
    vFragColor = texture(rectangleImage, vVaryingTexCoord);
}

