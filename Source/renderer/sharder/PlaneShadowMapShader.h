#pragma once

namespace MoonGlare::Renderer {

struct PlaneShadowMapShaderDescriptor {
    enum class InLayout {
        Position,
        TextureUV,
        Normal,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        CameraMatrix,
        ModelMatrix,
        MaxValue,
    };
    enum class Sampler {
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        //switch (s) {
        //default:
        return nullptr;
        //}
    }
};

}
