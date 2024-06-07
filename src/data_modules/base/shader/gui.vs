#include "/shader/common.glsl"
#include "/shader/gui_common.glsl"

out vec2 Texture0UV;
out vec3 VertexColor;

void main() {
    vec4 vpos = (uCameraMatrix * uModelMatrix) * vec4(inVertexPosition, 1.0);
    gl_Position = vpos;

    Texture0UV = inTexture0UV;
    VertexColor = inVertexColor;
    // if (gTileMode.x == 0) {
    // outVertexPosition.x = TexCoord.x;
    // } else {
    //     outVertexPosition.x = Pos.x / gPanelSize.x;
    // }
    // if (gTileMode.y == 0) {
    // outVertexPosition.y = TexCoord.y;
    // } else {
    //     outVertexPosition.y = Pos.y / gPanelSize.y;
    // }
}
