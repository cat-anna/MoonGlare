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
};

} //namespace MoonGlare::Renderer::Configuration
