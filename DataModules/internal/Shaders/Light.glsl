//-----------LIGHT-COMMON-----------

struct BaseLight_t {
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

vec4 CalcLightInternal(BaseLight_t BaseLight, vec3 LightDirection, vec3 WorldPos, vec3 Normal) {
    vec4 AmbientColor = vec4(BaseLight.Color, 1.0) * BaseLight.AmbientIntensity;
    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
	
	float gSpecularPower = 0.0;
	float gMatSpecularIntensity = 0.0;

	DiffuseFactor *= BaseLight.DiffuseIntensity;
    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(BaseLight.Color, 1.0) * DiffuseFactor;

        vec3 VertexToEye = normalize(CameraPos - WorldPos);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        SpecularFactor = pow(SpecularFactor, gSpecularPower);
        if (SpecularFactor > 0.0) {
            SpecularColor = vec4(BaseLight.Color, 1.0) * gMatSpecularIntensity * SpecularFactor;
        }
    }

    return (AmbientColor + DiffuseColor + SpecularColor);
};

//-----------LIGHT-ATTENUATION-----------

struct Attenuation_t {
	float Constant;
    float Linear;
    float Exp;
	float MinThreshold;
};

float CalcAttenuation(Attenuation_t att, float Distance) {
    float Attv = att.Constant + att.Linear * Distance +
				 att.Exp * (Distance * Distance);
	return min(1.0 / Attv, att.MinThreshold);
}

//-----------LIGHT-POINT-----------

struct PointLight_t {
	BaseLight_t	Base;
    vec3 Position; 
    Attenuation_t Atten;
};
uniform PointLight_t PointLight;

vec4 CalcPointLight(vec3 WorldPos, vec3 Normal) {
    vec3 LightDirection = WorldPos - PointLight.Position;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 Color = CalcLightInternal(PointLight.Base, LightDirection, WorldPos, Normal);
    Color.xyz *= CalcAttenuation(PointLight.Atten, Distance);
	return Color;
};

//-----------LIGHT-DIRECTIONAL-----------

struct DirectionalLight_t {
 	BaseLight_t	Base;
   vec3 Direction;
};
uniform DirectionalLight_t DirectionalLight;

vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal) {
    return CalcLightInternal(DirectionalLight.Base, DirectionalLight.Direction, WorldPos, Normal);
};

//-----------LIGHT-SPOT-----------

struct SpotLight_t {
 	BaseLight_t	Base;
	vec3 Direction;
    vec3 Position; 
    Attenuation_t Atten;	
	float CutOff;
};
uniform SpotLight_t SpotLight;

vec4 CalcSpotLight(vec3 WorldPos, vec3 Normal) {
	vec3 LightToWord = WorldPos - SpotLight.Position;
	float Distance = length(LightToWord);

	vec3 LightToPixel = normalize(LightToWord);
	float SpotFactor = dot(normalize(-SpotLight.Direction), LightToPixel);

	if (SpotFactor > SpotLight.CutOff) {
		vec4 Color = CalcLightInternal(SpotLight.Base, -SpotLight.Direction, WorldPos, Normal);
		Color.xyz *= CalcAttenuation(SpotLight.Atten, Distance);	
		Color.xyz *= (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - SpotLight.CutOff));
		Color.a = 1.0f;
		return Color;
	}
	else 
		discard;
}
