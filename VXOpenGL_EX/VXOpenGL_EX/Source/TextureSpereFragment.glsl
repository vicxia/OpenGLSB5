#version 410

out vec4 vFragColor;

uniform vec4       ambientColor;
uniform vec4       diffuseColor;
uniform vec4       specularColor;
uniform sampler2D  colorMap;

smooth in vec3 vVaryingNormal;
smooth in vec3 vVaryingLightDir;
smooth in vec2 vTexCoords;

void main(void)
{
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));
    vFragColor = diff * diffuseColor;
    
    vFragColor += ambientColor;
    
    vFragColor *= texture(colorMap, vTexCoords);
    
    if (diff != 0) {
        vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
        float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
        float fSpec = pow(spec, 128.0);
        vFragColor.rgb += vec3(fSpec, fSpec, fSpec);
    }
}
