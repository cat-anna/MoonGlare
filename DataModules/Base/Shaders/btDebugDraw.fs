//#include "Common.glsl"

in vec3 VertexColor;
out vec4 FragColor;

void main() {
	//WorldPosOut = FSin.VertexPosition.xyz;
	FragColor = vec4(VertexColor, 1.0);
};
