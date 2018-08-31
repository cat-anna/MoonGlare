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

        //ShadowMapSize,
        //EnableShadows,

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
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        case Uniform::ModelMatrix: return "ModelMatrix";
        case Uniform::CameraPos: return "CameraPos";

            //case Uniform::ShadowMapSize: return "ShadowMapSize";
            //case Uniform::EnableShadows: return "EnableShadowTest";

        case Uniform::Color: return "PointLight.Base.Color";
        case Uniform::AmbientIntensity: return "PointLight.Base.AmbientIntensity";
        case Uniform::DiffuseIntensity: return "PointLight.Base.DiffuseIntensity";
        case Uniform::Position: return "PointLight.Position";

        case Uniform::Attenuation: return "PointLight.Attenuation";

        case Uniform::ScreenSize: return "ScreenSize";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::PositionMap: return "gPositionMap";
        case Sampler::ColorMap: return "gColorMap";
        case Sampler::NormalMap: return "gNormalMap";
        default: return nullptr;
        }
    }
};

}