#include "Deferred/Common.glsl"

layout(location = 0) in vec3 Position;

void main() {
    gl_Position = (CameraMatrix * ModelMatrix) * vec4(Position, 1.0);
}
