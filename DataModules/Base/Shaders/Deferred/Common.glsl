#include "Common.glsl"
#include "StaticFog.glsl"
#include "Light.glsl"
#include "ShadowTest.glsl"
#include "Gamma.glsl"

//-----------------------------------------------

uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;

//-----------------------------------------------

uniform PointLight_t gPointLight;

//-----------------------------------------------

vec2 CalcTexCoord(vec4 Pos) {
    return vec2(Pos) / ScreenSize;
}

//-----------------------------------------------
