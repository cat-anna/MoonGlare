
//-----------MATERIAL-----------

// uniform sampler2D gDiffuseMap;
// uniform sampler2D gNormalMap;
// uniform sampler2D gShinessMap;
// uniform sampler2D gSpecularMap;
// uniform sampler2D gEmissiveMap;

// struct Material_t {
// 	vec3 diffuseColor;
// 	vec3 specularColor;
// 	vec3 emissiveColor;
// 	float shiness;
// 	float opacity;
// };

//-----------Global-Uniforms-----------

uniform vec2 uViewportSize;

uniform float uTimeDelta;
uniform float uGlobalTime;

uniform mat4 uModelMatrix;
uniform mat4 uCameraMatrix;

// uniform vec3 uCameraPos;
// uniform mat4 WorldMatrix;

//-----------Default-samplers-----------

uniform sampler2D sTexture2dSampler0;
// uniform sampler2D sTexture2dSampler1;
// uniform sampler2D sTexture2dSampler2;
// uniform sampler2D sTexture2dSampler4;

//-----------Inputs-----------

// NOTE: must match with shader_resource_interface.hpp
layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec2 inTexture0UV;
layout(location = 2) in vec3 inVertexColor;

// layout(location = 3) in vec2 inTexture1UV;
// layout(location = 4) in vec2 inTexture2UV;
// layout(location = 5) in vec2 inTexture3UV;
