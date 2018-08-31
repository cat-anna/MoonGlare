#include "Deferred/Common.glsl"

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;

out vec2 VertexUV0;
out vec3 VertexNormal;
out vec3 VertexWorldPos;
out mat3 TBN;

void main() {
    gl_Position = (CameraMatrix * ModelMatrix) * vec4(Position, 1.0);
    VertexUV0 = TexCoord;

   vec3 Bitangent = cross(Normal, Tangent);
   vec3 T = normalize(vec3(ModelMatrix * vec4(Tangent,   0.0)));
   vec3 B = normalize(vec3(ModelMatrix * vec4(Bitangent, 0.0)));
   vec3 N = normalize(vec3(ModelMatrix * vec4(Normal,    0.0)));
   TBN = mat3(T, B, N);

    // if(!gUseNormalMap) {
	    VertexNormal = normalize((
            ModelMatrix * 
            vec4(Normal, 0.0)).xyz);
    // }
	VertexWorldPos = (ModelMatrix * vec4(Position, 1.0)).xyz;
}
