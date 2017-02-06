#include "Common.glsl"

layout(location = 0) in vec3 Pos;
layout(location = 3) in vec3 Color;

out vec4 VertexPosition;

void main() {
	vec4 vpos = WorldMatrix * vec4(Pos, 1.0);

	gl_Position = vpos;
	VertexPosition = vpos;
};
