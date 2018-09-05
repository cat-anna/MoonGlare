
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;

out vec2 VertexUV0;

void main() {
    gl_Position = /*(CameraMatrix * ModelMatrix) * */ vec4(Position, 1.0);
    VertexUV0 = TexCoord;
}
