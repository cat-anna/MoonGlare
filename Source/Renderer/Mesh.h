#pragma once

namespace MoonGlare::Renderer {

struct alignas(16) Mesh { 
    bool valid;
    uint8_t __padding[3];

    uint16_t elementMode;            //TODO: is this needed, is GL_TRIANGLES required? (no?)
    uint16_t indexElementType;

    uint16_t baseVertex;
    uint16_t baseIndex;

    uint32_t numIndices;
};
static_assert(sizeof(Mesh) == 16);
static_assert(std::is_pod_v<Mesh>);

}
