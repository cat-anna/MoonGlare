/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:46:28,36
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "Configuration.Renderer.h"
#include "iRenderDevice.h"
#include "nfRenderer.h"

#include "TextureRenderTask.h"
#include <Foundation/InterfaceMap.h>
#include <Foundation/Tools/PerfViewClient.h>

namespace MoonGlare::Renderer {

class RendererFacade;

class alignas(16) RenderDevice final : public iRenderDevice, protected Tools::PerfView::PerfProducer {
    using ThisClass = RenderDevice;
    using Conf = Configuration::FrameBuffer;
public:
    RenderDevice(InterfaceMap &ifaceMap);

    bool Initialize(RendererFacade *renderer);
    bool Finalize();
    
    Frame* NextFrame();
    void Submit(Frame *frame);
    void ReleaseFrame(Frame *frame);
    Frame* PendingFrame();

    bool IsFramePending() const { return m_PendingFrame.load() != nullptr; }

    void Step();
    void ProcessPendingCtrlQueues();

    TextureRenderTask* AllocateTextureRenderTask() {
        assert(this);
        return m_UnusedTextureRender.pop(nullptr);
    }

    bool CommitControlCommandQueue(Commands::CommitCommandQueue *queue) {
        Commands::CommitCommandQueue* null = nullptr;
        return m_CommitCommandQueue.compare_exchange_weak(null, queue);
    }

    uint64_t FrameCounter() const { return frameCouter; }
    void SetCaptureScreenShoot() override { captureScreenShoot = true; }
private:
    std::array<Memory::aligned_ptr<Frame>, Conf::Count> m_Frames;
    std::atomic<uint32_t> m_FreeFrameBuffers = 0;
    std::atomic<Frame*> m_PendingFrame = nullptr;
    std::atomic<Commands::CommitCommandQueue*> m_CommitCommandQueue = nullptr;
    uint64_t frameCouter = 0;
    std::atomic<bool> captureScreenShoot = false;

    Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit> m_UnusedTextureRender;
    std::array<TextureRenderTask, Configuration::TextureRenderTask::Limit> m_TextureRenderTask;

    RendererFacade *m_RendererFacade = nullptr;

    DeclarePerformanceCounter(DroppedFrames);
    DeclarePerformanceCounter(FramesProcessed);
    
    void ProcessFrame(Frame *frame);
    void CaptureScreenshoot(MoonGlare::Renderer::iContext * Ctx);
};

} //namespace MoonGlare::Renderer
