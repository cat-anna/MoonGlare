#include "Common.glsl"

 uniform sampler2D Texture0;

in vec2 VertexUV0;
in vec3 VertexNormal;
in vec4 VertexWorldPos;
in vec4 VertexPosition;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(texture2D(Texture0, VertexUV0));
	FragColor.xyz *= Material.BackColor;
    //FragColor.x = 1;
    //FragColor.a = 1.0f;
};
