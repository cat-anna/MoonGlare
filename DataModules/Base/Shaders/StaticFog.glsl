//MoonGlare engine shader

struct StaticFog_t {
	bool Enabled;
	vec3 Color;
	
	float Start;
	float End;
};

uniform StaticFog_t gStaticFog;

float CalcStaticFogFactor(float Distance) {
	if(!gStaticFog.Enabled) {
		return 1.0f;
	 }
	
	float factor = (gStaticFog.End - Distance) / (gStaticFog.End - gStaticFog.Start); 
	return clamp(factor, 0.0, 1.0);
}

vec3 CalcStaticShadow(vec3 WorldPos, vec3 fcolor) {
	float Distance = length(WorldPos - CameraPos);
	float factor = CalcStaticFogFactor(Distance);
	return mix(gStaticFog.Color, fcolor, factor);
}
	// float Distance =1.0 - length(WorldPos) / 10;
//	float factor = (5 - Distance) / (5 - 1); 
	
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

//	vec3 fogc = vec3(1,1,1)*0.1;
//	factor = clamp(factor, 0.0, 1.0);
//	return mix(fogc, fcolor, factor);
	// return fcolor * fogc /*gStaticFog.Color*/ * factor;
	//vec3(0,1,1) /*gStaticFog.Color*/ * factor;
