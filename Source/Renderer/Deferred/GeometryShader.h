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
        SpecularColor,
        EmissiveColor,
        ShinessExponent,

        UseNormalMap,

        MaxValue,
    };
    enum class Sampler {
        DiffuseMap,
        NormalMap,
        ShinessMap,
        SpecularMap,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";

        case Uniform::DiffuseColor: return "gMaterial.diffuseColor";
        case Uniform::SpecularColor: return "gMaterial.specularColor";
        case Uniform::EmissiveColor: return "gMaterial.emissiveColor";
        case Uniform::ShinessExponent: return "gMaterial.shinessExponent";

        case Uniform::UseNormalMap: return "gUseNormalMap";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::DiffuseMap: return "gDiffuseMap";
        case Sampler::NormalMap: return "gNormalMap";
        case Sampler::ShinessMap: return "gShinessMap";
        case Sampler::SpecularMap: return "gSpecularMap";
        default: return nullptr;
        }
    }
};

}
