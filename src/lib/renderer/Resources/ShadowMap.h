#pragma once

#include "../Commands/CommandQueue.h"
#include "../Configuration.Renderer.h"

namespace MoonGlare::Renderer {

struct ShadowMap {
    Device::TextureHandle textureHandle;
    Device::FramebufferHandle framebufferHandle;

    bool Valid() const {
        return textureHandle != Device::InvalidTextureHandle && framebufferHandle != Device::InvalidFramebufferHandle;
    }

    void InitPlane(Commands::CommandQueue &q, const Configuration::Shadow &conf);
    void InitCube(Commands::CommandQueue &q, const Configuration::Shadow &conf);
};
//static_assert((sizeof(PlaneShadowMap) % 16) == 0, "Invalid size!");

}
