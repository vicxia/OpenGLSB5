#version 410

in vec4 vVertex;
uniform mat4 mvpMatrix;

out vec3 vVaryingTexCoord;

void main(void)
{
    vVaryingTexCoord = normalize(vVertex.xyz);
    gl_Position = mvpMatrix * vVertex;
}
