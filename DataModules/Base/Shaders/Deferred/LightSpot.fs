#include "Deferred/Common.glsl"

out vec4 FragColor;

void main() {
    vec2 TexCoord = CalcTexCoord(gl_FragCoord);
	vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
	vec3 Color = texture(gColorMap, TexCoord).xyz;
	vec3 Normal = texture(gNormalMap, TexCoord).xyz;
	//Normal = normalize(Normal);

	FragColor = CalcSpotLight(WorldPos, Normal, vec4(Color, 1));

	if(EnableShadowTest) {
		FragColor.xyz *= PlanarShadowTest(WorldPos, Normal);
	}

	FragColor.xyz = CalcStaticShadow(WorldPos, FragColor.xyz);
	FragColor.xyz = pow(FragColor.xyz, vec3(1.0/2.2));

//    FragColor.xy = TexCoord;
}
