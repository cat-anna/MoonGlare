#include "/shader/common.glsl"
#include "/shader/gui_common.glsl"

out vec4 FragColor;
in vec2 TextureUV;
in vec3 CurrentColor;

void main() {
    FragColor = vec4(1, 1, 1, 1); //  pow(FragColor.xyz, vec3(1.0/2.2));
}
