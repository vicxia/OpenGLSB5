#version 410
in vec4 vVertex;
in vec3 vNormal;

uniform vec4 diffuseColor;
uniform vec3 vLightPosition;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

smooth out vec4 vVaryingColor;

void main(void)
{
    vec3 vEyeNormal = normalMatrix * vNormal;
    vec4 vPosition4 = mvMatrix * vVertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;
//    vec3 vLightDir = normalize(vLightPosition - vPosition3); //spot light
    vec3 vLightDir = normalize(vLightPosition); //directional light
    float diff = max(0.0, dot(vEyeNormal, vLightDir));
    vVaryingColor.xyz = diff * diffuseColor.xyz;
    vVaryingColor.a = 1.0;
    gl_Position = mvpMatrix * vVertex;
}

























