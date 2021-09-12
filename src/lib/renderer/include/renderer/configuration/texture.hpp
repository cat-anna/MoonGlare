#pragma once

#include "renderer/types.hpp"
#include <array>

namespace MoonGlare::Renderer::Configuration {

struct Texture {
    static constexpr size_t kLimit = 1024;
    static constexpr size_t kStandbyPoolSize = 128;

    enum class Filtering : uint8_t {
        Nearest,
        Linear,
        Bilinear,
        Trilinear,

        MaxValue,
        UseGlobal,
        Default = Bilinear,
    };

    enum class Edges : uint8_t {
        Repeat,
        Clamp,

        MaxValue,
        Default = Clamp,
    };

    //     enum class ChannelSwizzle : uint8_t {
    //         R,
    //         G,
    //         B,
    //         A,
    //         Zero,
    //         One,
    //     };

    //     static inline uint16_t ChannelSwizzleToEnum(ChannelSwizzle cs) {
    //         switch (cs) {
    //         case ChannelSwizzle::R:
    //             return GL_RED;
    //         case ChannelSwizzle::G:
    //             return GL_GREEN;
    //         case ChannelSwizzle::B:
    //             return GL_BLUE;
    //         case ChannelSwizzle::A:
    //             return GL_ALPHA;
    //         case ChannelSwizzle::Zero:
    //             return GL_ZERO;
    //         case ChannelSwizzle::One:
    //             return GL_ONE;
    //         default:
    //             __debugbreak();
    //         }
    //     }

    //     union ColorSwizzle {
    //         struct {
    //             ChannelSwizzle R : 3;
    //             ChannelSwizzle G : 3;
    //             ChannelSwizzle B : 3;
    //             ChannelSwizzle A : 3;
    //             bool enable : 1;
    //         };
    //         uint16_t m_UIntValue;

    //         ColorSwizzle &operator=(const ColorSwizzle &) = default;
    //         ColorSwizzle &operator=(ChannelSwizzle cs) {
    //             R = G = B = A = cs;
    //             return *this;
    //         }
    //     };
    //     static_assert(sizeof(ColorSwizzle) == sizeof(uint16_t), "Invalid size!");

    //     static ColorSwizzle MakeColorSwizzle(uint16_t v) {
    //         ColorSwizzle cs;
    //         cs.m_UIntValue = v;
    //         return cs;
    //     }
};

struct TextureGlobalConfig {
    Texture::Filtering filtering = Texture::Filtering::Default;
};

struct TextureLoadConfig {
    Texture::Filtering filtering = Texture::Filtering::UseGlobal;
    Texture::Edges edges = Texture::Edges::Clamp;

    // bool generate_mip_maps : 1 = true;
    // bool srgb_color_space : 1 = false;

    // Conf::ColorSwizzle m_Swizzle;
    // static_assert(sizeof(Flags) == sizeof(uint8_t), "Invalid size!");

    // static TextureLoad Default() {
    //     Flags f;
    //     f.m_UIntValue = 0;
    //     f.generateMipMaps = true;
    //     return {
    //         Conf::Filtering::Default,
    //         Conf::Edges::Default,
    //         Conf::MakeColorSwizzle(0),
    //         f,
    //     };
    // }

    void Check(const TextureGlobalConfig &global) {
        if (filtering == Texture::Filtering::UseGlobal) {
            filtering = global.filtering;
        }
    }
};

// static_assert(sizeof(TextureLoad) == sizeof(uint64_t), "Invalid size"); //allowed to be 64bits if necessary
// static_assert(std::is_pod_v<TextureLoad>, "Must be pod");

// struct TextureRenderTask {
//     static constexpr uint32_t Limit = 64;
// };

} // namespace MoonGlare::Renderer::Configuration
