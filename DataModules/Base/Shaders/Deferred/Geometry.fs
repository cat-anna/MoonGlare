#include "Deferred/Common.glsl"

in vec2 VertexUV0;
in vec3 VertexNormal;
in vec3 VertexWorldPos;

layout(location = 1) out vec3 WorldPosOut;
layout(location = 2) out vec3 DiffuseOut;
layout(location = 3) out vec3 NormalOut;

void main() {
    vec4 color = texture2D(Texture0, VertexUV0);
    if(color.a < 0.1) { //Material.AlphaThreshold
    	discard;
    }
    DiffuseOut	= color.xyz * Material.BackColor;
    WorldPosOut	= VertexWorldPos;
    NormalOut	= VertexNormal;
}
