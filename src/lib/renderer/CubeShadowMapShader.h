#pragma once

namespace MoonGlare::Renderer {

struct CubeShadowMapShaderDescriptor {
    enum class InLayout {
        Position,
        TextureUV,
        Normal,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        CameraMatrix0,
        CameraMatrix1,
        CameraMatrix2,
        CameraMatrix3,
        CameraMatrix4,
        CameraMatrix5,
        ModelMatrix,  
        LightPosition,
        MaxValue,
    };
    enum class Sampler {
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix0: return "CameraMatrices[0]";
        case Uniform::CameraMatrix1: return "CameraMatrices[1]";
        case Uniform::CameraMatrix2: return "CameraMatrices[2]";
        case Uniform::CameraMatrix3: return "CameraMatrices[3]";
        case Uniform::CameraMatrix4: return "CameraMatrices[4]";
        case Uniform::CameraMatrix5: return "CameraMatrices[5]";
        case Uniform::LightPosition: return "LightPosition";
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
