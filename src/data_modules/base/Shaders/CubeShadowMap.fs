#include "Common.glsl"

in vec4 FragPos;

uniform vec3 LightPosition;
// uniform float far_plane;

void main() {

    float lightDistance = length(FragPos.xyz - LightPosition);

    float far_plane = 100.0f;
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
};
