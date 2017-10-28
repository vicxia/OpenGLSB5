#version 410

out vec4 vFragColor;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform sampler2D colorMap0;
uniform sampler2D colorMap1;
uniform float dissolveFactor;

smooth in vec3 vVaryingNormal;
smooth in vec3 vVaryingLightDir;
smooth in vec2 vVaryingTexCoords;

void main(void)
{
    vec4 vCloudSample = texture(colorMap1, vVaryingTexCoords);
    if (vCloudSample.r < dissolveFactor) {
        discard;
    }
    
    float diff = max(0.0, dot(normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    vFragColor = diff * diffuseColor;
    vFragColor += ambientColor;
    vec4 vTexColor = texture(colorMap0, vVaryingTexCoords);
    vFragColor *= vTexColor;
    
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir),
                                         normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if (diff != 0) {
        float fSpec = pow(spec, 128.0);
        vFragColor.rgb += vec3(fSpec, fSpec, fSpec);
    }
}



































