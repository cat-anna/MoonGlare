#include "Gamma.glsl"

uniform sampler2D gCurrentFrameMap;

in vec2 VertexUV0;

out vec4 FragColor;

void main() {
	vec3 Color = texture(gCurrentFrameMap, VertexUV0).xyz;
	Color.xyz = ApplyGamma(Color.xyz);

    // Color.xyz = vec3(1.0) - Color.xyz;

    vec2 offset = 1.0 / textureSize(gCurrentFrameMap, 0);  

  vec2 offsets[9] = vec2[](
        vec2(-offset.x,  offset.y), // top-left
        vec2( 0.0f,    offset.y), // top-center
        vec2( offset.x,  offset.y), // top-right
        vec2(-offset.x,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset.x,  0.0f),   // center-right
        vec2(-offset.x, -offset.y), // bottom-left
        vec2( 0.0f,   -offset.y), // bottom-center
        vec2( offset.x, -offset.y)  // bottom-right    
    );

//   float kernel[9] = float[]( //particular sharpen
//         -1, -1, -1,
//         -1,  9, -1,
//         -1, -1, -1
//     );
// float kernel[9] = float[](//blur
//     1.0 / 16, 2.0 / 16, 1.0 / 16,
//     2.0 / 16, 4.0 / 16, 2.0 / 16,
//     1.0 / 16, 2.0 / 16, 1.0 / 16  
// );

// float kernel[9] = float[](//tst
//         0, -0.5, 0,
//         -0.5, 4, -0.5,
//         0, -0.5, 0
// );

// float kernel[9] = float[](//tst2
//         -1, 0, 1,
//         -1, 1, 1,
//         -1, 0, 1
// );

float kernel[9] = float[](//norm
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
);

// float kernel[9] = float[](//norm
//         0, 0.1, 0,
//         0.1, 0.6, 0.1,
//         0, 0.1, 0
// );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(gCurrentFrameMap, VertexUV0.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    FragColor = vec4(ApplyGamma(col), 1.0);
}
