
#if 0

uniform float gGamma;
uniform float gGammaInverted;
uniform bool gGammaEnabled;

vec3 ApplyGamma(vec3 val) {
	 if(gGamma <= 0)
		return val;
	vec3 r = pow(val, vec3(gGammaInverted));
	return r;
}

#endif
