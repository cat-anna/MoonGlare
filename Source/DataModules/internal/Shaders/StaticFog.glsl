//MoonGlare engine shader

struct StaticFog_t {
	bool Enabled;
	vec3 Color;
	
	float Start;
	float End;
	
	float Density;
};

uniform StaticFog_t gStaticFog;

vec3 CalcStaticShadow(vec3 WorldPos) {
	if(!gStaticFog.Enabled) {
		return vec3(1.0);
	}
	
	float Distance = length(WorldPos - CameraPos);
	float factor = (gStaticFog.End - Distance) / (gStaticFog.End - gStaticFog.Start); 
	
// float getFogFactor(FogParameters params, float fFogCoord) 
// { 
   // float fResult = 0.0; 
   // if(params.iEquation == 0) 
      // fResult = (params.fEnd-fFogCoord)/(params.fEnd-params.fStart); 
   // else if(params.iEquation == 1) 
      // fResult = exp(-params.fDensity*fFogCoord); 
   // else if(params.iEquation == 2) 
      // fResult = exp(-pow(params.fDensity*fFogCoord, 2.0)); 
   // fResult = 1.0-clamp(fResult, 0.0, 1.0); 
   // return fResult; 
// }

// vec4 ApplyStaticFog(in vec4 BaseColor, in vec4 Position) {
	// float dist = length(Position.xyz);// work out distance from camera to point
		// get a fog factor (thickness of fog) based on the distance
	// float fog_fac = (dist - StaticFog.Start) / (StaticFog.End - StaticFog.Start);
	// return vec4(mix (BaseColor.rgb, StaticFog.Color, fog_fac),BaseColor.a);
// }

	factor = clamp(factor, 0.0, 1.0);
	return gStaticFog.Color * factor;
}
