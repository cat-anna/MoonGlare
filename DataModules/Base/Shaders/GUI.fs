#include "Common.glsl"
#include "GUICommon.glsl"

vec4 ProcessBaseColor(vec4 fragment) {
	fragment.xyz *= gBaseColor.a ;//* fragment.a;
	fragment.xyz *= gBaseColor.xyz;
	fragment.a *= gBaseColor.a;
	return fragment;
}

//---------------------------------------------------------------------------------------

float ProcessTile(float pos, int TileMode, float Border, int frameCnt, int frameindex) {
	if(TileMode == 0) {
		if(frameCnt != 0) {
			float shard = 1.0f / float(frameCnt);
			float base = shard * frameindex;
			return base + pos * shard;
		}
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
	tex.x = ProcessTile(VertexPosition.x, gTileMode.x, gPanelBorder / gPanelAspect, gFrameCount.x, gFrameIndex.x);
	tex.y = ProcessTile(VertexPosition.y, gTileMode.y, gPanelBorder,                gFrameCount.y, gFrameCount.y - gFrameIndex.y - 1);

	FragColor = ProcessBaseColor(texture2D(Texture0, tex));
	FragColor.xyz = pow(FragColor.xyz, vec3(1.0/2.2));
}
