#include "Deferred/Common.glsl"

in vec2 VertexUV0;
in vec3 VertexNormal;
in vec3 VertexWorldPos;
in mat3 TBN;

layout(location = 1) out vec4 WorldPosOut;
layout(location = 2) out vec4 DiffuseOut;
layout(location = 3) out vec4 NormalOut;

void main() {
    vec4 color = texture2D(gTexture0, VertexUV0);
    if(color.a < 0.1) { //Material.AlphaThreshold
    	discard;
    }
    DiffuseOut	= vec4(color.xyz * gMaterial.backColor, 0);
    WorldPosOut	= vec4(VertexWorldPos, 1);

    if(gUseNormalMap) {
        // vec3 n = texture2D(gNormalMap, VertexUV0).rgb;
        // n = normalize(n * 2.0 - vec3(1));
        // NormalOut = (ModelMatrix * vec4(n, 0)).xyz;

        vec3 normal = texture(gNormalMap, VertexUV0).rgb;
        normal = normalize(normal * 2.0 - vec3(1.0));   
        normal = normalize(TBN * normal); 

        NormalOut = vec4(normal, 0);
        // DiffuseOut = normal;
    } 
    else {
        // DiffuseOut = VertexNormal; 
        NormalOut = vec4(VertexNormal, 0);
        // DiffuseOut = vec3(0,1,0);
    }
}
