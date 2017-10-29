#version 410

in vec4 vVertex;
in vec3 vNormal;

smooth out float textureCoordinate;

uniform vec3 vLightPosition;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main(void)
{
    vec3 vEyeNormal = normalMatrix * vNormal;
    vec4 vPosition4 = mvMatrix * vVertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;
    vec3 vLightDir = normalize(vLightPosition - vPosition3);
    textureCoordinate = max(0.0, dot(vLightDir, vEyeNormal));
    gl_Position = mvpMatrix * vVertex;
}
