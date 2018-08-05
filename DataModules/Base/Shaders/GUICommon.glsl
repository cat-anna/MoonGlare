
uniform sampler2D Texture0;

uniform vec4 gBaseColor;
uniform vec2 gPanelSize;

uniform float gPanelAspect;
uniform ivec2 gTileMode;
uniform float gPanelBorder;
uniform ivec2 gFrameCount;
uniform ivec2 gFrameIndex;

const float TextureDivs = 3.0f;
const float TextureRatio = 1.0f / TextureDivs;
