#include "Deferred/Common.glsl"

in vec2 VertexUV0;
in vec3 VertexNormal;
in vec3 VertexWorldPos;
in mat3 TBN;

uniform Material_t gMaterial;

layout(location = 1) out vec4 WorldPosOut;
layout(location = 2) out vec4 DiffuseOut;
layout(location = 3) out vec4 NormalOut;
layout(location = 4) out vec4 SpecularOut;

uniform bool gUseNormalMap;

void main() {
    vec4 diffuseColor = texture2D(gDiffuseMap, VertexUV0);
    vec4 specularColor = texture2D(gSpecularMap, VertexUV0);
    float shinessValue = gMaterial.shinessExponent;
    
    shinessValue *= texture2D(gShinessMap, VertexUV0).r;
    // shinessValue = shinessValue;

    diffuseColor.xyz *= gMaterial.diffuseColor;
    specularColor.xyz *= gMaterial.specularColor;
    // specularColor.xyz = gMaterial.specularColor;

    diffuseColor.a = 1.0f;
    specularColor.a = 1.0f;
    // specularColor.g = shinessValue;
    // diffuseColor.xyz = vec3(shinessValue);
    //emissiveColor
// SHINESS_SCALER
    DiffuseOut	= diffuseColor;
    SpecularOut = specularColor;
    WorldPosOut	= vec4(VertexWorldPos, 1);

    if(gUseNormalMap) {
        vec3 normal = texture(gNormalMap, VertexUV0).rgb;
        normal = normalize(normal * 2.0 - vec3(1.0));   
        normal = normalize(TBN * normal); 

        NormalOut = vec4(normal, 0);
        // DiffuseOut = normal;
    } 
    else {
        NormalOut = vec4(VertexNormal, 0);
        // DiffuseOut = VertexNormal; 
    }
    NormalOut.a = shinessValue;
}
