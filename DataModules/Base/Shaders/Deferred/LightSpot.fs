#include "Deferred/Common.glsl"
#include "PlaneShadowTest.glsl"
#include "Light.glsl"

uniform SpotLight_t gSpotLight;

out vec4 FragColor;

void main() {
	vec2 TexCoord = CalcTexCoord(gl_FragCoord);

	vec3 worldPos = texture(gPositionMap, TexCoord).xyz;
	vec4 Color = texture(gColorMap, TexCoord);
	vec4 specularColor = texture(gSpecularMap, TexCoord);
	vec4 emissiveColor = texture(gEmissiveMap, TexCoord);
	vec4 NormalShiness = texture(gNormalMap, TexCoord);

	vec3 normal = NormalShiness.xyz;
	float shiness = NormalShiness.a;
//	Normal = normalize(Normal);

	Material_t mat;
	mat.diffuseColor = Color.xyz;
	mat.specularColor = specularColor.xyz;
	mat.emissiveColor = emissiveColor.xyz;
	mat.shinessExponent = shiness;
	mat.opacity = 1.0f;

	float shadow = PlaneShadowTest(worldPos);
	FragColor = CalcSpotLight(worldPos, normal, gSpotLight, mat, shadow);
}
