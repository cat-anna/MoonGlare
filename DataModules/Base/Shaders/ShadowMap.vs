#include "Common.glsl"

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextureUV;
layout(location = 2) in vec3 Normal;

void main() {
	gl_Position = (CameraMatrix * ModelMatrix) * vec4(Position, 1.0);
};
