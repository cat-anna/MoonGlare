#include "Common.glsl"

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Color;

out vec3 VertexColor;

void main() {
	vec4 vpos = CameraMatrix * vec4(Pos, 1.0);

	gl_Position = vpos;
    VertexColor = Color;
};
