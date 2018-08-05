#include "Common.glsl"
#include "GUICommon.glsl"

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TexCoord;

out vec2 VertexPosition;

void main() {
	vec4 vpos = (CameraMatrix * ModelMatrix) * vec4(Pos, 1.0);
	gl_Position = vpos;
	if ( gTileMode.x == 0 ) {
		VertexPosition.x = TexCoord.x;
	} else {
		VertexPosition.x = Pos.x / gPanelSize.x;
	}
	if ( gTileMode.y == 0 ) {
		VertexPosition.y = TexCoord.y;
	} else {
		VertexPosition.y = Pos.y / gPanelSize.y;
	}
}
