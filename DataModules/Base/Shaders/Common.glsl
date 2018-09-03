
//-----------MATERIAL-----------

struct Material_t {
	float alphaThreshold;
	vec3 backColor;
};

uniform Material_t gMaterial;

//-----------Global-Uniforms-----------

uniform vec2 ScreenSize;
uniform vec3 CameraPos;
uniform mat4 WorldMatrix;
uniform mat4 ModelMatrix;
uniform mat4 CameraMatrix;
