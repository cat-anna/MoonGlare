//-----------LIGHT-COMMON-----------

struct BaseLight_t {
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

vec4 CalcLightInternal(BaseLight_t BaseLight, vec3 LightDirection, vec3 WorldPos, vec3 Normal) {
    vec4 AmbientColor  = vec4(0, 0, 0, 0);
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    AmbientColor = vec4(BaseLight.Color, 1.0) * BaseLight.AmbientIntensity;

	float gSpecularPower = 1;
    float gMatSpecularIntensity = 0.01;

    float DiffuseFactor = dot(Normal, -LightDirection);
	DiffuseFactor *= BaseLight.DiffuseIntensity;
    if (DiffuseFactor > 0.0) {
        DiffuseColor = 
			vec4(BaseLight.Color, 1.0)
		 * DiffuseFactor;

        vec3 VertexToEye = normalize(CameraPos - WorldPos);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        SpecularFactor = pow(SpecularFactor, gSpecularPower);
        if (SpecularFactor > 0.0) {
            SpecularColor = vec4(BaseLight.Color, 1.0) * gMatSpecularIntensity * SpecularFactor;
        }
    }

    vec4 result = AmbientColor + DiffuseColor + SpecularColor;
	result.a = 1.0;
	return result;
};

//-----------LIGHT-ATTENUATION-----------

float CalcAttenuation(vec4 att, float Distance) {
    float Attv = att[0] +			              //constant
				 att[1] * Distance +              //linerar
				 att[2] * (Distance * Distance);  //exp

	if(Attv <= 0)
		return att[3];
	return 1.0f / Attv;
//	return pow(Attv, -2.2);// min(1.0 / Attv, att.MinThreshold);
}

//-----------LIGHT-POINT-----------

struct PointLight_t {
	BaseLight_t	Base;
    vec3 Position;
    vec4 Attenuation;
};
uniform PointLight_t PointLight;

vec4 CalcPointLight(vec3 WorldPos, vec3 Normal) {
	vec3 LightToWord = WorldPos - PointLight.Position;
    float Distance = length(LightToWord);
	vec3 LightToPixel = normalize(LightToWord);

    vec4 Color = CalcLightInternal(PointLight.Base, LightToPixel, WorldPos, Normal);
    Color.xyz *= CalcAttenuation(PointLight.Attenuation, Distance);
//	Color.xyz = pow(Color.xyz, vec3(1.0/2.2));
	return Color;
};

//-----------LIGHT-DIRECTIONAL-----------

struct DirectionalLight_t {
 	BaseLight_t	Base;
	vec3 Direction;
};
uniform DirectionalLight_t DirectionalLight;

vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal) {
    vec4 Color = CalcLightInternal(DirectionalLight.Base, DirectionalLight.Direction, WorldPos, Normal);
//	Color.xyz = pow(Color.xyz, vec3(1.0/2.2));
	return Color;
};

//-----------LIGHT-SPOT-----------

struct SpotLight_t {
 	BaseLight_t	Base;
	vec3 Direction;
    vec3 Position;
    vec4 Attenuation;
	float CutOff;
};
uniform SpotLight_t SpotLight;

vec4 CalcSpotLight(vec3 WorldPos, vec3 Normal) {
	vec3 LightToWord = WorldPos - SpotLight.Position;

	vec3 LightToPixel = normalize(LightToWord);
	float SpotFactor = dot(normalize(-SpotLight.Direction), LightToPixel);

	if (SpotFactor > SpotLight.CutOff) {
		float Distance = length(LightToWord);
		vec4 Color = CalcLightInternal(SpotLight.Base, LightToPixel, WorldPos, Normal);
        Color.xyz *= CalcAttenuation(SpotLight.Attenuation, Distance);
		Color.xyz *= (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - SpotLight.CutOff));
		//Color.xyz = LightToPixel;

//		Color.xyz = pow(Color.xyz, vec3(1.0/2.2));

//		Color.a = 1.0f;
		//Color.g = 1.0f;
		//	Color.b = SpotFactor;
		return Color;
	}
	//else
	//	discard;

	return vec4(0.0);
}
