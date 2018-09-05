
//-----------MATERIAL-----------

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;
uniform sampler2D gShinessMap;
uniform sampler2D gSpecularMap;
uniform sampler2D gEmissiveMap;

struct Material_t {
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 emissiveColor;
	float shinessExponent;
	float opacity;
};

//-----------Global-Uniforms-----------

uniform vec2 ScreenSize;
uniform vec3 CameraPos;
uniform mat4 WorldMatrix;
uniform mat4 ModelMatrix;
uniform mat4 CameraMatrix;
