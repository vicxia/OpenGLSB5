#version 410

in vec4 vVertex;
in vec3 vNormal;
in vec4 vTexture0;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform vec3 vLightPosition;

smooth out vec3 vVaryingNormal;
smooth out vec3 vVaryingLightDir;
smooth out vec2 vTexCoords;

void main(void)
{
    vVaryingNormal = normalMatrix * vNormal;
    
    vec4 vPosition4 = mvMatrix * vVertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;
    vVaryingLightDir = normalize(vLightPosition - vPosition3);
    
    vTexCoords = vTexture0.st;
    
    gl_Position = mvpMatrix * vVertex;
}
