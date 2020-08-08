#include "Common.glsl"
#include "StaticFog.glsl"

//-----------------------------------------------

uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;

//-----------------------------------------------
vec2 CalcTexCoord(vec4 Pos) {
    return vec2(Pos) / ScreenSize;
}

//-----------------------------------------------
