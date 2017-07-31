#include "Common.glsl"
#include "StaticFog.glsl"
#include "Light.glsl"
#include "ShadowTest.glsl"
#include "Gamma.glsl"

//-----------------------------------------------

uniform sampler2D Texture0;
uniform sampler2D PositionMap;
uniform sampler2D ColorMap;
uniform sampler2D NormalMap;

//-----------------------------------------------

vec2 CalcTexCoord(vec4 Pos) {
    return vec2(Pos) / ScreenSize;
}

//-----------------------------------------------
