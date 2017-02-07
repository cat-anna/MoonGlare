#include "Common.glsl"

uniform sampler2D Texture0;
uniform float gVisibility;

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

out vec2 VertexUV0;

void main() {
	vec4 vpos = (CameraMatrix * ModelMatrix) * vec4(Pos, 1.0);

	gl_Position = vpos;
	// VSout.VertexPosition = vpos;
    VertexUV0 = TexCoord;
	// VSout.VertexNormal = (ModelMatrix * vec4(Normal, 0.0)).xyz;
	// VSout.VertexWorldPos = (ModelMatrix * vec4(Pos, 1.0));
};
