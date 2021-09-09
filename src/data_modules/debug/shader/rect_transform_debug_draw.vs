#include "/shader/common.glsl"
#include "/shader/gui_common.glsl"

layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec2 inTextureUV;
layout(location = 2) in vec3 inVertexColor;

out vec2 TextureUV;
out vec3 CurrentColor;

void main() {
    vec4 vpos = (uCameraMatrix * uModelMatrix) * vec4(inVertexPosition, 1.0);
    gl_Position = vpos;

    TextureUV = inTextureUV;
    CurrentColor = inVertexColor;
}
