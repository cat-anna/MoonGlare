
uniform mat4 gLightMatrix;
uniform bool gEnableShadowTest;
uniform sampler2D gPlaneShadowMap;

float PlaneShadowTest(vec3 worldPos) {
	if(!gEnableShadowTest)
		return 1.0f;

	vec4 UVinShadowMap = (gLightMatrix * vec4(worldPos, 1.0));
	vec3 ShadowCoord = UVinShadowMap.xyz / UVinShadowMap.w;
	ShadowCoord = (ShadowCoord+1)/2;

	// float bias = 1e-3;
	float bias = 0.0005;
	float m = 0;

	ivec2 msize = textureSize(gPlaneShadowMap, 0);
	vec2 ShadowPixelSize = vec2(1.0 / msize[0], 1.0 / msize[1]);

	for(int i = 0; i < GAUSSIAN_DISC_SIZE; ++i) {
		vec2 sampledelta = ShadowPixelSize * GAUSSIAN_DISC[i];
		vec2 samplepos = vec2(ShadowCoord) + sampledelta;

	 	float s = texture(gPlaneShadowMap, samplepos).z;
	 	if(s < ShadowCoord.z - bias)
	 		m += 1;
	}

	return 1.0f - m / float(GAUSSIAN_DISC_SIZE);
}
