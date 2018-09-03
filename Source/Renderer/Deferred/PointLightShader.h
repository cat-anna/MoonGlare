#pragma once

namespace MoonGlare::Renderer::Deferred {

struct PointLightShaderDescriptor {
    enum class InLayout {
        Position,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        CameraMatrix,
        ModelMatrix,
        CameraPos,

        EnableShadows,

        Color,
        AmbientIntensity,
        DiffuseIntensity,
        Position,

        Attenuation,

        ScreenSize,

        MaxValue,
    };
    enum class Sampler {
        Unused,
        PositionMap,
        ColorMap,
        NormalMap,
        SpecularMap,
        ShadowMap,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        case Uniform::CameraPos: return "CameraPos";

        case Uniform::EnableShadows: return "EnableShadowTest";

        case Uniform::Color: return "gPointLight.Base.Color";
        case Uniform::AmbientIntensity: return "gPointLight.Base.AmbientIntensity";
        case Uniform::DiffuseIntensity: return "gPointLight.Base.DiffuseIntensity";
        case Uniform::Position: return "gPointLight.Position";

        case Uniform::Attenuation: return "gPointLight.Attenuation";

        case Uniform::ScreenSize: return "ScreenSize";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::PositionMap: return "gPositionMap";
        case Sampler::ColorMap: return "gColorMap";
        case Sampler::NormalMap: return "gNormalMap";
        case Sampler::SpecularMap: return "gSpecularMap";
        case Sampler::ShadowMap: return "gCubeShadowMap";
        default: return nullptr;
        }
    }
};

}