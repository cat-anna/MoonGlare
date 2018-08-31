#pragma once

namespace MoonGlare::Renderer::Deferred {

struct GeometryShaderDescriptor {
    enum class InLayout {
        Position,
        TextureUV,
        Normal,
        Tangent,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        CameraMatrix,
        ModelMatrix,
        DiffuseColor,

        UseNormalMap,

        MaxValue,
    };
    enum class Sampler {
        DiffuseMap,
        NormalMap,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        case Uniform::DiffuseColor: return "gMaterial.backColor";
        case Uniform::UseNormalMap: return "gUseNormalMap";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::DiffuseMap: return "gTexture0";
        case Sampler::NormalMap: return "gNormalMap";
        default: return nullptr;
        }
    }
};

}
