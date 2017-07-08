
uniform mat4 LightMatrix;
uniform vec2 ShadowMapSize;
uniform bool EnableShadowTest;

//----------------------------------

uniform sampler2D PlaneShadowMap;

float PlanarShadowTest(vec3 WorldPos, vec3 Normal) {
	vec4 UVinShadowMap = (LightMatrix * vec4(WorldPos, 1.0));
	vec3 ShadowCoord = UVinShadowMap.xyz / UVinShadowMap.w;
	ShadowCoord = (ShadowCoord+1)/2;

	float bias = 1e-5;
	float s;
	float m = 0;//5;

	vec2 ShadowPixelSize = vec2(1.0 / ShadowMapSize[0], 1.0 / ShadowMapSize[1]);

	const int delta = 1;
	for(int i = -delta; i <= delta; ++i) {
		for(int j = -delta; j <= delta; ++j) {
				vec2 sampledelta = ShadowPixelSize * vec2(i, j);
				vec2 samplepos = vec2(ShadowCoord) + sampledelta;

				float match = 1;

				s = texture(PlaneShadowMap, samplepos).z;
				if(s < ShadowCoord.z - bias)
					m += match;
		}
	}

	m = 1.0 - m / pow(delta*2+1, 2);

	if (m < 0)
		m = 0;
		//discard;
	return m;
}
