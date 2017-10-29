
#version 410

out vec4 vFragColor;
uniform samplerCube cubeMap;
uniform sampler2D tarnishMap;

in vec3 vVaryingTexCoord;
in vec2 vTarnishCoords;

void main(void)
{
    vFragColor = texture(cubeMap, vVaryingTexCoord);
    vFragColor *= texture(tarnishMap, vTarnishCoords);
}
