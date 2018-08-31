#include "Deferred/Common.glsl"

in vec2 VertexUV0;
in vec3 VertexNormal;
in vec3 VertexWorldPos;
in mat3 TBN;

layout(location = 1) out vec3 WorldPosOut;
layout(location = 2) out vec3 DiffuseOut;
layout(location = 3) out vec3 NormalOut;

void main() {
    vec4 color = texture2D(gTexture0, VertexUV0);
    if(color.a < 0.1) { //Material.AlphaThreshold
    	discard;
    }
    DiffuseOut	= color.xyz * gMaterial.backColor;
    WorldPosOut	= VertexWorldPos;

    if(gUseNormalMap) {
        // vec3 n = texture2D(gNormalMap, VertexUV0).rgb;
        // n = normalize(n * 2.0 - vec3(1));
        // NormalOut = (ModelMatrix * vec4(n, 0)).xyz;

        vec3 normal = texture(gNormalMap, VertexUV0).rgb;
        normal = normalize(normal * 2.0 - vec3(1.0));   
        normal = normalize(TBN * normal); 

        NormalOut = normal;
        // DiffuseOut = normal;
    } 
    else {
        // DiffuseOut = VertexNormal; 
        NormalOut = VertexNormal;
        // DiffuseOut = vec3(0,1,0);
    }
}
