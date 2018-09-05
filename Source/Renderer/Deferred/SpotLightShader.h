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
        EmissiveMap,
        ShadowMap,
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        case Uniform::CameraPos: return "CameraPos";

        case Uniform::LightMatrix: return "gLightMatrix";

        case Uniform::EnableShadows: return "gEnableShadowTest";

        case Uniform::Color: return "gSpotLight.Base.Color";
        case Uniform::AmbientIntensity: return "gSpotLight.Base.AmbientIntensity";
        case Uniform::DiffuseIntensity: return "gSpotLight.Base.DiffuseIntensity";
        case Uniform::Position: return "gSpotLight.Position";
        case Uniform::Direction: return "gSpotLight.Direction";
        case Uniform::CutOff: return "gSpotLight.CutOff";
        case Uniform::Attenuation: return "gSpotLight.Attenuation";

        case Uniform::ScreenSize: return "ScreenSize";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::PositionMap: return "gPositionMap";
        case Sampler::ColorMap: return "gColorMap";
        case Sampler::NormalMap: return "gNormalMap";
        case Sampler::EmissiveMap: return "gEmissiveMap";
        case Sampler::ShadowMap: return "gPlaneShadowMap";
        default: return nullptr;
        }
    }
};

}
