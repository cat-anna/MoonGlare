
uniform mat4 LightMatrix;
uniform vec2 ShadowMapSize;
uniform bool EnableShadowTest;

//----------------------------------

uniform sampler2D PlaneShadowMap;

/*
limit = 16
radius = 2

gaussian = { [[...]] }

function rand2() 
	return math.random(), math.random()
end

function rand() 
	return table.remove(gaussian), table.remove(gaussian)
end

for i=1,limit do

	local a,r = rand2()   
    a = a * 2 * math.pi
    r = r * radius
    
    local x = r * math.sin(a)
    local y = r * -math.cos(a)

   print(string.format([[vec2(%f,%f),]], x,y))
end

*/

#define DISC_SIZE 16
uniform vec2 DISC[DISC_SIZE] = { 
	vec2(0.008861,-1.127101),
	vec2(1.515866,-0.564125),
	vec2(-0.488491,0.826095),
	vec2(1.447793,1.055790),
	vec2(-1.339600,-0.659553),
	vec2(1.526181,-0.788497),
	vec2(-0.995538,0.252414),
	vec2(0.028261,0.009971),
	vec2(0.395953,-0.611956),
	vec2(0.265085,-0.199531),
	vec2(-0.064379,-0.889029),
	vec2(0.006353,-0.006844),
	vec2(0.042292,-0.754553),
	vec2(-0.225650,1.119825),
	vec2(-0.724476,0.974498),
	vec2(1.146566,-0.666179)
};

float PlanarShadowTest(vec3 WorldPos, vec3 Normal) {
	vec4 UVinShadowMap = (LightMatrix * vec4(WorldPos, 1.0));
	vec3 ShadowCoord = UVinShadowMap.xyz / UVinShadowMap.w;
	ShadowCoord = (ShadowCoord+1)/2;

	float bias = 1e-5;
	// float bias = 0.005;
	float m = 0;

	vec2 ShadowPixelSize = vec2(1.0 / ShadowMapSize[0], 1.0 / ShadowMapSize[1]);

	for(int i = 0; i < GAUSSIAN_DISC_SIZE; ++i) {
		vec2 sampledelta = ShadowPixelSize * GAUSSIAN_DISC[i];
		vec2 samplepos = vec2(ShadowCoord) + sampledelta;

	 	float s = texture(PlaneShadowMap, samplepos).z;
	 	if(s < ShadowCoord.z - bias)
	 		m += 1;
	}

	return 1.0f - m / float(GAUSSIAN_DISC_SIZE);
}
