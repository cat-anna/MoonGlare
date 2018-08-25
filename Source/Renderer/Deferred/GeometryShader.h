#pragma once

namespace MoonGlare::Renderer::Deferred {

struct GeometryShaderDescriptor {
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
        DiffuseColor,

        MaxValue,
    };
    enum class Sampler {
        DiffuseMap,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        case Uniform::DiffuseColor: return "Material.BackColor";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::DiffuseMap: return "Texture0";
        default: return nullptr;
        }
    }
};

}
