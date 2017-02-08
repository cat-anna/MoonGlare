#include "Common.glsl"

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

void main() {
	gl_Position = (CameraMatrix * ModelMatrix) * vec4(Pos, 1.0);
};
