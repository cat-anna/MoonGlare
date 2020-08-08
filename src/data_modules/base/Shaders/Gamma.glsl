
uniform float gGamma;
uniform float gGammaInverted;
uniform bool gGammaEnabled;

vec3 ApplyGamma(vec3 val) {
	val = pow(val, 
	// vec3(gGammaInverted)
		vec3(1.0/2.2)
	);
	return val;
}
