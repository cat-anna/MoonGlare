#include "Deferred/Common.glsl"

out vec4 FragColor;

uniform samplerCube ShadowMap;

float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - PointLight.Position;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(ShadowMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= 100.0f;//far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.005; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}

void main() {
    vec2 TexCoord = CalcTexCoord(gl_FragCoord);
	vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
	vec3 Color = texture(gColorMap, TexCoord).xyz;
	vec3 Normal = texture(gNormalMap, TexCoord).xyz;
//	Normal = normalize(Normal);
	FragColor = CalcPointLight(WorldPos, Normal, vec4(Color, 1));

    float shadow = ShadowCalculation(WorldPos);                  
    // vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;   


	FragColor.xyz = (1.0 - shadow) *
	 CalcStaticShadow(WorldPos, FragColor.xyz);
	FragColor.xyz = pow(FragColor.xyz, vec3(1.0/2.2));
//    FragColor.xy=TexCoord;
//    FragColor.a=1.0f;
}
