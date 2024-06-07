#include "/shader/common.glsl"
#include "/shader/gui_common.glsl"

out vec2 TextureUV;
out vec3 CurrentColor;

void main() {
    vec4 vpos = (uCameraMatrix * uModelMatrix) * vec4(inVertexPosition, 1.0);
    gl_Position = vpos;

    TextureUV = inTexture0UV;
    CurrentColor = inVertexColor;
}
