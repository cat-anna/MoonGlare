
//-----------MATERIAL-----------

struct Material_t {
	float AlphaThreshold;
	vec3 BackColor;
};

uniform Material_t Material;

//-----------Global-Uniforms-----------

uniform vec2 ScreenSize;
uniform vec3 CameraPos;
uniform mat4 WorldMatrix;
uniform mat4 ModelMatrix;
