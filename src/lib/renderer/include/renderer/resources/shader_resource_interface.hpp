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
        kDummy,
        kMaxValue,
    };

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
        // case Sampler::DiffuseMap:
        //     return "gDiffuseMap";
        default:
            return nullptr;
        }
    }

    ShaderVariables() {
        uniform.fill(Device::kInvalidShaderUniformHandle);
        sampler.fill(Device::kInvalidShaderUniformHandle);
    }

    static const ShaderVariables &GetInvalidHandles() {
        static ShaderVariables variables;
        return variables;
    }

    Device::ShaderUniformHandle GetUniform(Uniform u) const {
        return uniform[static_cast<size_t>(u)];
    }

    std::array<Device::ShaderUniformHandle, static_cast<size_t>(Uniform::kMaxValue)> uniform;
    std::array<Device::ShaderUniformHandle, static_cast<size_t>(Sampler::kMaxValue)> sampler;
};

class iShaderResource {
public:
    virtual ~iShaderResource() = default;

    virtual void ReloadAllShaders() = 0;

    virtual ShaderHandle LoadShader(const std::string &name) = 0;
    virtual ShaderHandle LoadShader(FileResourceId resource_id) = 0;
    virtual const ShaderVariables *GetShaderVariables(ShaderHandle handle) = 0;
};

} // namespace MoonGlare::Renderer::Resources
