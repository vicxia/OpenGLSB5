#version 410

out vec4 vFragColor;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;

void main(void)
{
    float diff = max(0.0, dot(vVaryingNormal, vVaryingLightDir));
    vFragColor = diffuseColor * diff;
    vFragColor += ambientColor;
    if (diff != 0) {
        vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
        float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
        float fSpec = pow(spec, 128);
        vFragColor.xyz += vec3(fSpec, fSpec, fSpec);
    }
}
