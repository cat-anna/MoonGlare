#include "Common.glsl"

 uniform sampler2D Texture0;
//uniform sampler2DShadow Texture0;

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

out vec2 VertexUV0;
out vec3 VertexNormal;
out vec4 VertexWorldPos;
out vec4 VertexPosition;

void main() {
	vec4 vpos = (CameraMatrix * ModelMatrix) * vec4(Pos, 1.0);

	gl_Position = vpos;
	VertexPosition = vpos;
    VertexUV0 = TexCoord;
	VertexNormal = (ModelMatrix * vec4(Normal, 0.0)).xyz;
	VertexWorldPos = (ModelMatrix * vec4(Pos, 1.0));
};
