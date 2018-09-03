#include "Common.glsl"

// layout(location = 0) in vec3 Position;
// layout(location = 1) in vec2 TextureUV;
// layout(location = 2) in vec3 Normal;

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 CameraMatrices[6];
// uniform vec3 LightPosition;

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main() {
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = CameraMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
};
