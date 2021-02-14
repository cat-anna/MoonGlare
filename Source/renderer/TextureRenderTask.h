#pragma once

#include "Commands/CommandQueue.h"
#include "nfRenderer.h"
#include <math/EigenMath.h>

namespace MoonGlare::Renderer {

class alignas(16) TextureRenderTask final {
public:
    TextureRenderTask();
    ~TextureRenderTask();

    bool Initialize();
    bool Finalize();

    void Begin();
    void End();

    void SetFrame(Frame *frame) { m_Frame = frame; }
    void SetTarget(TextureResourceHandle &handle, emath::ivec2 Size);

    Commands::CommandQueue &GetCommandQueue() { return m_CommandQueue; }
    template <typename CMD, typename... ARGS> typename CMD::Argument *PushCommand(ARGS &&... args) {
        return m_CommandQueue.PushCommand<CMD>(std::forward<ARGS>(args)...);
    }

private:
    TextureResourceHandle m_TargetTexture;
    Device::FramebufferHandle m_Framebuffer;
    uint32_t padding;
    emath::ivec2 m_Size;
    Frame *m_Frame;
    void *padding2;

    Commands::CommandQueue m_CommandQueue;
};

static_assert((sizeof(TextureRenderTask) & 0xF) == 0, "Invalid size!");

} // namespace MoonGlare::Renderer
