#pragma once

namespace MoonGlare::Renderer::Deferred {

struct SpotLightShaderDescriptor {
    enum class InLayout {
        Position,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        CameraMatrix,
        ModelMatrix,
        LightMatrix,
        CameraPos,


        Color,
        AmbientIntensity,
        DiffuseIntensity,
        EnableShadows,
        Position,
        Direction,
        CutOff,

        Attenuation,

        ScreenSize,

        MaxValue,
    };
    enum class Sampler {
        Unused,
        PositionMap,
        ColorMap,
        NormalMap,
        PlaneShadowMap,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        case Uniform::CameraPos: return "CameraPos";

        case Uniform::LightMatrix: return "LightMatrix";

        case Uniform::EnableShadows: return "EnableShadowTest";

        case Uniform::Color: return "SpotLight.Base.Color";
        case Uniform::AmbientIntensity: return "SpotLight.Base.AmbientIntensity";
        case Uniform::DiffuseIntensity: return "SpotLight.Base.DiffuseIntensity";
        case Uniform::Position: return "SpotLight.Position";
        case Uniform::Direction: return "SpotLight.Direction";
        case Uniform::CutOff: return "SpotLight.CutOff";

        case Uniform::Attenuation: return "SpotLight.Attenuation";

        case Uniform::ScreenSize: return "ScreenSize";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::PositionMap: return "gPositionMap";
        case Sampler::ColorMap: return "gColorMap";
        case Sampler::NormalMap: return "gNormalMap";
        case Sampler::PlaneShadowMap: return "PlaneShadowMap";
        default: return nullptr;
        }
    }
};

}
