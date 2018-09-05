#pragma once

namespace MoonGlare::Renderer {

struct PostProcessShader {
    enum class InLayout {
        Position,
        TextureUV,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        __unused,
        MaxValue,
    };
    enum class Sampler {
        Unused,
        CurrentFrame,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        default:
        case Sampler::CurrentFrame: return "gCurrentFrameMap";
        return nullptr;
        }
    }
};

}
