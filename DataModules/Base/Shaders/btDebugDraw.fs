#include "Common.glsl"

in vec4 VertexPosition;
out vec4 FragColor;

void main() {
	//WorldPosOut = FSin.VertexPosition.xyz;
	FragColor = vec4( Material.BackColor, 1.0);
};
