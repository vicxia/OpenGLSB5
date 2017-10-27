#version 410

in vec4 vVertex;
in vec3 vNormal;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
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
    vec3 vLightDir = normalize(vLightPosition - vPosition3);
    float diff = max(0, dot(vEyeNormal, vLightDir));
    vVaryingColor = diffuseColor * diff;
    vVaryingColor += ambientColor;
    vec3 vReflection = normalize(reflect(-vLightDir, vEyeNormal));
    float spec = max(0.0, dot(vEyeNormal, vReflection));
    if (spec != 0) {
        float fSpec = pow(spec, 128);
        vVaryingColor.xyz += vec3(fSpec, fSpec, fSpec);
//        vVaryingColor.xyz += specularColor.xyz * fSpec;
    }
    gl_Position = mvpMatrix * vVertex;
}
