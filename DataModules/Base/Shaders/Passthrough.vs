
uniform mat4 gModelMatrix;
uniform mat4 gCameraMatrix;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextureUV;
//layout(location = 2) in vec3 Normal;

out vec2 VertexUV0;

void main() {
	vec4 vpos = (gCameraMatrix * gModelMatrix) * vec4(Position, 1.0);
	gl_Position = vpos;
    VertexUV0 = TextureUV;
};
