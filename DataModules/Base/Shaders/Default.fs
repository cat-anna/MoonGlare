#include "Common.glsl"

 uniform sampler2D Texture0;
//uniform sampler2DShadow Texture0;

in vec2 VertexUV0;
in vec3 VertexNormal;
in vec4 VertexWorldPos;
in vec4 VertexPosition;

out vec4 FragColor;

void main() {
	 FragColor = vec4(texture2D(Texture0, VertexUV0));
	// FragColor = vec4(1);
	// vec3 pos = vec3(5, 0.5, -0.5);
	// vec3 v = vec3();
	// FragColor.x = texture( Texture0, v );
// FragColor=vec4(0, v, 0,  1);
	// if(FragColor.a < Material.AlphaThreshold)
	//	 discard;
	FragColor.xyz *= Material.BackColor;

	//FragColor = ApplyStaticFog(FragColor, VertexPosition);
};
