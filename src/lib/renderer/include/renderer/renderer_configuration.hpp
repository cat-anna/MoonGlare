#pragma once

#include "renderer/configuration/texture.hpp"
#include "types.hpp"
#include <array>

namespace MoonGlare::Renderer::Configuration {

struct Requirement {
    //ogl
    // static constexpr uint32_t kOpenGLVersionMajor = 4;
    // static constexpr uint32_t kOpenGLVersionMinor = 2;

    //system
    // static constexpr uint32_t kCPUCores = 2;
};

// struct FrameResourceStorage {
//     static constexpr uint32_t TextureLimit = 64;
//     static constexpr uint32_t VAOLimit = 64;
//     static constexpr uint32_t PlaneShadowMapLimit = 64;
//     static constexpr uint32_t CubeShadowMapLimit = 64;
// };

struct FrameBuffer {
    static constexpr size_t kCount = 3;
};

struct RenderTarget {
    static constexpr size_t kMemoryPoolSize = 8 * 1024 * 1024;
    static constexpr size_t kDynamicBufferPoolSize = 64;
    static constexpr size_t kDynamicVaoPoolSize = 32;

    // static constexpr uint32_t SubQueueCount = 64;

    enum class RenderLayer {
        // Controll,
        // PreRender, //TextureRenderTask and friends
        // PlaneShadowMaps,
        // CubeShadowMaps, //temporary
        //Render,
        // DeferredGeometry,
        // DeferredLighting, //
        // PostRender,       //splitted to allow debugdraw
        // //Postprocess,
        // GUI, //temporary

        kMaxValue,
    };
};

// struct CommandBucket {
//     static constexpr uint32_t MaxCommandsPerBucket = 16;
//     using BucketCommandCount = uint8_t;
//     using BucketCommandIndex = uint16_t;
//     static constexpr uint32_t MaxBuckets = 1024;
//     static constexpr uint32_t MaxCommands = 4096;
//     static constexpr uint32_t ArgumentMemoryBuffer = 1 * 256 * 1024;
// };

struct CommandQueue {
    static constexpr size_t kCommandLimit = 4096 * 8;
    static constexpr size_t kArgumentMemoryBuffer = 1 * 1024 * 1024;
    //     static constexpr uint32_t BytesPerCommand = ArgumentMemoryBuffer / CommandLimit;
};

// struct IndexBuffer {
//     static constexpr uint32_t TextureBuffer = 256;
// };

// struct Resources {
// template <size_t SIZE>
// using BitmapAllocator = Memory::LinearAtomicBitmapAllocator<SIZE, uint32_t, uint32_t>;

// static constexpr uint32_t AsyncQueueCount = 2;
// static constexpr uint32_t QueueMemory = 32 * 1024 * 1024;
// };

//---------------------------------------------------------------------------------------

struct Shader {
    static constexpr size_t kLimit = 16;
    static constexpr uint32_t kUniformLimit = 32;

    // using UniformLocations = std::array<Device::ShaderUniformHandle, UniformLimit>;

    // uint16_t gaussianDiscLength;
    // float gaussianDiscRadius;

    void ResetToDefault() {
        // gaussianDiscLength = 32;
        // gaussianDiscRadius = 0.5f;
    }
};

//---------------------------------------------------------------------------------------

struct Material {
    // static constexpr uint32_t Limit = 1024;
};

//---------------------------------------------------------------------------------------

struct Shadow {
    // bool enableShadows;
    // uint16_t shadowMapSize;

    void ResetToDefault() {
        //     enableShadows = true;
        //     shadowMapSize = 1024;
    }
};

//---------------------------------------------------------------------------------------

struct RendererConfiguration {
    TextureGlobalConfig texture;
    // Shadow shadow;
    // Shader shader;

    // float gammaCorrection = 2.2f;
};

// inline void to_json(nlohmann::json &j, const RendererConfiguration &p) {
// j = {
// {"value", p.value},
//     };
// }

// inline void from_json(const nlohmann::json &j, RendererConfiguration &p) {
// j.at("value").get_to(p.value);
// }

} // namespace MoonGlare::Renderer::Configuration
