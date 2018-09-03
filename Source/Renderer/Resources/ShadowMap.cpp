#include "../Commands/CommandQueue.h"
#include "../Commands/OpenGL/TextureCommands.h"
#include "ShadowMap.h"

namespace MoonGlare::Renderer {

void ShadowMap::InitPlane(Commands::CommandQueue &q, const Configuration::Shadow &conf) {
    auto *cmd = q.PushCommand<Commands::InitPlaneShadowMap>();
    cmd->size = static_cast<GLsizei>(conf.shadowMapSize);
    cmd->bufferHandle = &framebufferHandle;
    cmd->textureHandle = &textureHandle;
}

void ShadowMap::InitCube(Commands::CommandQueue &q, const Configuration::Shadow &conf) {
    auto *cmd = q.PushCommand<Commands::InitCubeShadowMap>();
    cmd->size = static_cast<GLsizei>(conf.shadowMapSize);
    cmd->bufferHandle = &framebufferHandle;
    cmd->textureHandle = &textureHandle;
}   

}
