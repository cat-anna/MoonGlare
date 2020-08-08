
uniform vec4 gBackColor;

uniform sampler2D Texture0;

in vec2 VertexUV0;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture2D(Texture0, VertexUV0));
	FragColor.xyz *= gBackColor.xyz;
    FragColor.a *= gBackColor.a;
};
