#pragma once

#include "Handles.h"
#include "Device/Types.h"

namespace MoonGlare::Renderer::Configuration {

struct Requirement {
    //ogl
    static constexpr uint32_t OpenGLVersionMajor = 4;
    static constexpr uint32_t OpenGLVersionMinor = 2;
    
    //system 
    static constexpr uint32_t CPUCores = 2;
};

struct FrameResourceStorage {
    static constexpr uint32_t TextureLimit = 64;
    static constexpr uint32_t VAOLimit = 64;
    static constexpr uint32_t PlaneShadowMapLimit = 64;
};

struct FrameBuffer {
    static constexpr uint32_t MemorySize = 1 * 128 * 1024;
    static constexpr uint32_t Count = 3;
    static constexpr uint32_t SubQueueCount = 64;

    enum class Layer {
        Controll,
        PreRender, //TextureRenderTask and friends
        ShadowMaps,
        //Render,
        DefferedGeometry,
        //DefferedLighting,
        //PostRender,
        //Postprocess,

        GUI, //temporary

        MaxValue,
    };
};

struct Context {
    enum class Window {
        First,
        //Second,
        //Third,
        //Fourth,

        MaxValue,
    };
};

struct CommandBucket {
    static constexpr uint32_t MaxCommandsPerBucket = 16;
    using BucketCommandCount = uint8_t;
    using BucketCommandIndex = uint16_t;
    static constexpr uint32_t MaxBuckets = 1024;
    static constexpr uint32_t MaxCommands = 4096;

    static constexpr uint32_t ArgumentMemoryBuffer = 1 * 128 * 1024;
};

struct CommandQueue {
    static constexpr uint32_t ArgumentMemoryBuffer = 1 * 64 * 1024; 
    static constexpr uint32_t CommandLimit = 4096;
    static constexpr uint32_t BytesPerCommand = ArgumentMemoryBuffer / CommandLimit;
};

struct IndexBuffer {
    static constexpr uint32_t TextureBuffer = 256;
};
struct Resources {
    template<size_t SIZE>
    using BitmapAllocator = ::Space::Memory::LinearAtomicBitmapAllocator<SIZE, uint32_t, uint32_t>;

    static constexpr uint32_t AsyncQueueCount = 2;
    static constexpr uint32_t QueueMemory = 10 * 1024 * 1024;
};

//---------------------------------------------------------------------------------------

struct Texture {
    static constexpr uint32_t Limit = 1024;
    static constexpr uint32_t Initial = 64;

    enum class Filtering : uint8_t {
        Nearest,
        Linear,
        Bilinear,
        Trilinear,

        MaxValue,
        Default = MaxValue,
    };

    enum class Edges : uint8_t {
        Repeat,
        Clamp,
        MaxValue,
        Default = MaxValue,
    };

    enum class ChannelSwizzle : uint8_t {
        R, G, B, A,
    };

    union ColorSwizzle {
        struct {
            ChannelSwizzle R : 2;
            ChannelSwizzle G : 2;
            ChannelSwizzle B : 2;
            ChannelSwizzle A : 2;
        };
        uint8_t m_UIntValue;
    };
    static_assert(sizeof(ColorSwizzle) == sizeof(uint8_t), "Invalid size!");

    static ColorSwizzle MakeColorSwizzle(uint8_t v) {
        ColorSwizzle cs;
        cs.m_UIntValue = v;
        return cs;
    }

    Filtering m_Filtering;

    void ResetToDefault() {
        m_Filtering = Filtering::Bilinear;
    }
};

struct TextureLoad {
    using Conf = Texture;
    Conf::Filtering m_Filtering;
    Conf::Edges m_Edges;
    Conf::ColorSwizzle m_Swizzle;

    union Flags {
        struct {
            bool m_Swizzle : 1;
        };
        uint8_t m_UIntValue;
    } m_Flags;

    static_assert(sizeof(Flags) == sizeof(uint8_t), "Invalid size!");

    static TextureLoad Default() {
        return {
            Conf::Filtering::Default,
            Conf::Edges::Default,
            Conf::MakeColorSwizzle(0),
            0,
        };
    }
};
static_assert(sizeof(TextureLoad) == sizeof(uint32_t), "Invalid size");//allowed to be 64bits if necessary
static_assert(std::is_pod<TextureLoad>::value, "Must be pod");

struct TextureRenderTask {
    static constexpr uint32_t Limit = 64;
};

//---------------------------------------------------------------------------------------

struct VAO {
    static constexpr uint32_t VAOLimit = 1024;
    static constexpr uint32_t VAOInitial = 64;
    static constexpr uint32_t MaxBuffersPerVAO = 8;

    using VAOBuffers = std::array<Device::BufferHandle, MaxBuffersPerVAO>;

    using ChannelType = GLuint;

    enum class InputChannels : ChannelType {
        Vertex,
        Texture0,
        Normals,

        //Color, //unused

        Index = 4,

        MaxValue,
    };

    static_assert(static_cast<uint32_t>(InputChannels::MaxValue) <= MaxBuffersPerVAO, "mismatched buffer count!");
};

//---------------------------------------------------------------------------------------

struct Shader {
    static constexpr uint32_t Limit = 32;
    static constexpr uint32_t UniformLimit = 20;
    using UniformLocations = std::array<Device::ShaderUniformHandle, UniformLimit>;
};

//---------------------------------------------------------------------------------------

struct Material {
    static constexpr uint32_t Limit = 1024;

};

//---------------------------------------------------------------------------------------

struct Shadow {
    enum class ShadowMapSize : uint16_t {
        Disable,
        Low,
        Medium,
        High,

        MaxValue,
        Default = Medium,
    };

    std::underlying_type_t<ShadowMapSize> GetShadowMapSize() const {
        //dumb values, they are subject to tests
        switch (m_ShadowMapSize) {
        case ShadowMapSize::Disable: return 1;
        case ShadowMapSize::Low: return 256;
        case ShadowMapSize::Medium: return 512;
        case ShadowMapSize::High: return 1024;
        default:
            return static_cast<std::underlying_type_t<ShadowMapSize>>(m_ShadowMapSize);
        }
    }

    ShadowMapSize m_ShadowMapSize;

    void ResetToDefault() {
        m_ShadowMapSize = ShadowMapSize::Default;
    }
};

//---------------------------------------------------------------------------------------

struct RuntimeConfiguration {
    Texture m_Texture;
    Shadow m_Shadow;

    void ResetToDefault() {
        m_Texture.ResetToDefault();
        m_Shadow.ResetToDefault();
    }
};

} //namespace MoonGlare::Renderer::Configuration
