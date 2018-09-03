#include "Common.glsl"
#include "StaticFog.glsl"
#include "Light.glsl"
#include "ShadowTest.glsl"
#include "Gamma.glsl"

//-----------------------------------------------

uniform sampler2D gTexture0;
uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;
uniform sampler2D gNormalMap;
uniform bool gUseNormalMap;

//-----------------------------------------------

vec2 CalcTexCoord(vec4 Pos) {
    return vec2(Pos) / ScreenSize;
}

//-----------------------------------------------
