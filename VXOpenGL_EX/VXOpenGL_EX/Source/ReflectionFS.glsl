
#version 410

out vec4 vFragColor;
uniform samplerCube cubeMap;

in vec3 vVaryingTexCoord;

void main(void)
{
    vFragColor = texture(cubeMap, vVaryingTexCoord);
}
