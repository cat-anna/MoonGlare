#include "Deferred/Common.glsl"

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

out vec2 VertexUV0;
out vec3 VertexNormal;
out vec3 VertexWorldPos;

void main() {
    gl_Position = (CameraMatrix * ModelMatrix) * vec4(Position, 1.0);
    VertexUV0 = TexCoord;
	VertexNormal = (ModelMatrix * vec4(Normal, 0.0)).xyz;
	VertexWorldPos = (ModelMatrix * vec4(Position, 1.0)).xyz;
}
