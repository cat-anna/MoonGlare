#include "Common.glsl"
#include "GUICommon.glsl"

vec4 ProcessBaseColor(vec4 fragment) {
	fragment *= gBaseColor.a;
	fragment.xyz = fragment.xyz * gBaseColor.xyz;
	return fragment;
}

//---------------------------------------------------------------------------------------

float ProcessTile(float pos, int TileMode, float Border) {
	if(TileMode == 0) {
		return pos;
	} else {
		float InvBorder = 1.0f / Border;
		float BorderRatio = InvBorder / 3.0f;

		if (pos < Border) {
			return pos * BorderRatio;
		} else {
			if (pos >= 1.0f - Border){
				return (1.0f - pos) * BorderRatio;
			} else {
				float d = (pos - Border) / (1.0f - 2 * Border);
				if(TileMode < 0) {
					if(InvBorder > TextureDivs)
						d *= int(InvBorder) - TextureDivs + 1.0f;
				} else {
					d *= TileMode;
				}
				d -= int(d);

				return TextureRatio * ( 1.0f + d);
			}
		}
	}

	return pos;//bug catcher, shall not reach here
}

//---------------------------------------------------------------------------------------

out vec4 FragColor;
in vec2 VertexPosition;

void main() {
	vec2 tex;
	tex.x = ProcessTile(VertexPosition.x, gTileMode.x, gPanelBorder / gPanelAspect);
	tex.y = ProcessTile(VertexPosition.y, gTileMode.y, gPanelBorder);

	FragColor = ProcessBaseColor(vec4(texture2D(Texture0, tex)));
}
