#pragma once

#include "../Handles.h"
#include "../Device/Types.h"

namespace MoonGlare::Renderer::Configuration {

struct Mesh {
    static constexpr uint32_t Limit = 1024;

    enum class Channel : uint8_t {
        Vertex,
        UV0,
        Normal,
        Index,
        MaxValue,
    };

    static constexpr uint32_t ChannelCount = static_cast<uint32_t>(Channel::MaxValue);
    using VAOBuffers = std::array<Device::BufferHandle, ChannelCount>;

    static constexpr uint32_t SubMeshLimit = 4;

    struct alignas(16) SubMesh { //subject of change
        bool valid;
        uint8_t __padding[3];

        uint16_t elementMode; //TODO: is this needed, is GL_TRIANGLES required? (no?)
        uint16_t indexElementType;

        uint16_t baseVertex;
        uint16_t baseIndex;

        uint32_t numIndices;
    };
    static_assert(sizeof(SubMesh) == 16);

    using SubMeshArray = std::array<SubMesh, SubMeshLimit>;
};

} //namespace MoonGlare::Renderer::Configuration
