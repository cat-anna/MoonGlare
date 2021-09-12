#pragma once

#include "renderer/device_types.hpp"
#include "renderer/types.hpp"
#include "resource_id.hpp"
#include <array>
#include <stdexcept>

namespace MoonGlare::Renderer::Resources {

struct ShaderVariables {
    enum class Uniform {
        kCameraMatrix,
        kModelMatrix,
        kMaxValue,
    };

    enum class Sampler {
        kTexture2dSampler0,
        // kTexture2dSampler1,
        // kTexture2dSampler2,
        // kTexture2dSampler3,
        kMaxValue,
    };

    enum class Layout {
        // NOTE: must match with common.glsl
        kVertexPosition = 0,
        kTexture0UV = 1,
        kVertexColor = 2,
    };

    // enum class InputChannel : ChannelType {
    //     kVertex,
    //     kNormals,
    //     kTexture0,
    //     // kTexture1,
    //     // kTexture2,
    //     // kTexture3,
    //     // //Color, //unused
    //     // Index,
    //     // Tangents, //TODO: in reverse order directional light is not working

    //     kMaxValue,
    // };

    constexpr static const char *GetUniformName(Uniform u) {
        switch (u) {
        case Uniform::kCameraMatrix:
            return "uCameraMatrix";
        case Uniform::kModelMatrix:
            return "uModelMatrix";

        // case Uniform::BaseColor:
        //     return "gBaseColor";
        // case Uniform::TileMode:
        //     return "gTileMode";
        // case Uniform::Border:
        //     return "gPanelBorder";
        // case Uniform::PanelSize:
        //     return "gPanelSize";
        // case Uniform::PanelAspect:
        //     return "gPanelAspect";
        default:
            return nullptr;
        }
    }
    constexpr static const char *GetSamplerName(Sampler s) {
        switch (s) {
        case Sampler::kTexture2dSampler0:
            return "sTexture2dSampler0";
        default:
            return nullptr;
        }
    }

    ShaderVariables() {
        uniform.fill(Device::kInvalidShaderUniformHandle);
        // sampler.fill(Device::kInvalidShaderUniformHandle);
    }

    static const ShaderVariables &GetInvalidHandles() {
        static ShaderVariables variables;
        return variables;
    }

    Device::ShaderUniformHandle GetUniform(Uniform u) const {
        return uniform[static_cast<size_t>(u)];
    }

    std::array<Device::ShaderUniformHandle, static_cast<size_t>(Uniform::kMaxValue)> uniform;
    // std::array<Device::ShaderUniformHandle, static_cast<size_t>(Sampler::kMaxValue)> sampler;
};

class iShaderResource {
public:
    virtual ~iShaderResource() = default;

    virtual void ReloadAllShaders() = 0;

    // must match with iRuntimeResourceLoader (double override by real implementation)
    virtual ResourceHandle LoadShaderResource(FileResourceId file_id) = 0;

    virtual ShaderHandle LoadShader(const std::string &name) = 0;
    virtual const ShaderVariables *GetShaderVariables(ShaderHandle handle) = 0;
};

} // namespace MoonGlare::Renderer::Resources
