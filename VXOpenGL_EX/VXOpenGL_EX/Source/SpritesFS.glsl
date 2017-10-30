#version 410

out vec4 vFragColor;

in vec4 vStarColor;

uniform sampler2D starImage;

void main(void)
{
    vFragColor = texture(starImage, gl_PointCoord) * vStarColor;
}
