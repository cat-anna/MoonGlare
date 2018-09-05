#include "Deferred/Common.glsl"
#include "Light.glsl"

out vec4 FragColor;

void main() {
    vec2 TexCoord = CalcTexCoord(gl_FragCoord);
	vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
	vec3 Color = texture(gColorMap, TexCoord).xyz;//ColorMap
	vec3 Normal = texture(gNormalMap, TexCoord).xyz;
	//Normal = normalize(Normal);
	FragColor = CalcDirectionalLight(WorldPos, Normal, vec4(Color, 1));
}
