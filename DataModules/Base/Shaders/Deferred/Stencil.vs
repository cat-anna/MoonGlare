#include "Deferred/Common.glsl"

layout(location = 0) in vec3 Position;

void main() {
    gl_Position = WorldMatrix * vec4(Position, 1.0);
}
