#pragma once

#include "aligned_ptr.hpp"
#include "core/engine_time.hpp"
#include "core/stop_interface.hpp"
#include "device_context.hpp"
#include "renderer/render_target_interface.hpp"
#include "renderer_configuration.hpp"
#include "rendering_device_interface.hpp"
#include "resources.hpp"
#include "synchronized_task_queue.hpp"
#include <atomic>
#include <chrono>
#include <gsl/gsl>

namespace MoonGlare::Renderer {

class FrameBuffer;

class RenderingDevice final : public iRenderingDevice,
                              public iContextResourceLoader,
                              public iStopInterface {
public:
    RenderingDevice(gsl::not_null<iEngineTime *> engine_time,
                    gsl::not_null<iDeviceContext *> _device_context,
                    gsl::not_null<iDeviceWindow *> _main_window,
                    gsl::not_null<iResourceManager *> _resource_manager);
    ~RenderingDevice() override;

    void SetResourceManager(gsl::not_null<iResourceManager *> _resource_manager);

    //iStopInterface
    void Stop() override;

    //iContextResourceLoader
    void PushResourceTask(std::shared_ptr<iContextResourceTask> task) override {
        resource_tasks.Push(std::move(task));
    }

    //iRenderingDevice
    void EnterLoop() override;
    void NextFrame() override;
    void SubmitFrame() override;

    //iRenderingDeviceFacade
    iRenderTarget *GetDisplayRenderTarget() override;
    iResourceManager *GetResourceManager() override;

    // uint64_t FrameCounter() const { return frameCounter; }
    // void SetCaptureScreenShoot() override { captureScreenShoot = true; }

private:
    std::atomic<bool> can_work{false};
    using clock_t = std::chrono::steady_clock;

    struct FrameQueue {
        //TODO: this needs to be reworked for full atomicity

        FrameQueue();

        FrameBuffer *FetchRenderPending();
        FrameBuffer *FetchNext();
        void Release(FrameBuffer *frame);
        void SubmitCurrent();

        uint64_t DroppedFrames() const { return dropped_frames; }
        uint64_t ProcessedFrames() const { return processed_frames; }

    private:
        void Drop(FrameBuffer *frame) {
            if (frame != nullptr) {
                ++dropped_frames;
                Release(frame);
            }
        }
        std::atomic<FrameBuffer *> render_pending = nullptr;
        std::atomic<FrameBuffer *> current_frame = nullptr;
        std::array<std::atomic<FrameBuffer *>, Configuration::FrameBuffer::kCount * 2>
            standby_frames;
        uint64_t dropped_frames = 0;   //TODO: change to atomic?
        uint64_t processed_frames = 0; //TODO: change to atomic?
    };

    RenderTargetProxy render_proxy;
    FrameQueue frame_queue;

    // std::atomic<Commands::CommitCommandQueue *> m_CommitCommandQueue = nullptr;
    // uint64_t frameCounter = 0;
    // std::atomic<bool> captureScreenShoot = false;

    iDeviceContext *const device_context;
    iDeviceWindow *const main_window;
    iResourceManager *const resource_manager;
    std::array<aligned_ptr<FrameBuffer>, Configuration::FrameBuffer::kCount> allocated_frames;

    SynchronizedTaskQueue<std::shared_ptr<iContextResourceTask>> resource_tasks;

    void ExecuteResourceTasks();

    // Memory::StaticVector<TextureRenderTask *,
    //                      Configuration::TextureRenderTask::Limit>
    //     m_UnusedTextureRender;
    // std::array<TextureRenderTask, Configuration::TextureRenderTask::Limit>
    //     m_TextureRenderTask;

    // RendererFacade *m_RendererFacade = nullptr;

    // DeclarePerformanceCounter(DroppedFrames);
    // DeclarePerformanceCounter(FramesProcessed);

    // void ProcessFrame(Frame *frame);
    // void CaptureScreenshoot(MoonGlare::Renderer::iContext *Ctx);
};

} // namespace MoonGlare::Renderer

#if 0
#pragma once

#include <Foundation/Settings.h>
#include <Foundation/iFileSystem.h>
#include <interface_map.h>

#include "Configuration.Renderer.h"

#include "iRendererFacade.h"

#include "iContext.h"

namespace MoonGlare::Renderer {

class RendererFacade final : public iRendererFacade {
  public:
    RendererFacade(InterfaceMap &ifaceMap);
    ~RendererFacade();

    // iRendererFacade
    iContext *GetContext() override;
    // void SetConfiguration(Configuration::RuntimeConfiguration Configuration)
    // override;
    void Initialize(const ContextCreationInfo &ctxifo,
                    iFileSystem *fileSystem) override;
    void Finalize() override;

    /** Shall work on main thread; does not return until stopped */
    void EnterLoop();
    void Stop();
    bool CanWork() const { return m_CanWork; }

    //	Context* CreateContext(const ContextCreationInfo& ctxifo);

    RenderDevice *GetDevice() {
        assert(this);
        return m_Device.get();
    }
    Resources::ResourceManager *GetResourceManager() {
        assert(this);
        return m_ResourceManager.get();
    }

    iAsyncLoader *GetAsyncLoader();

    template <typename T> void SetStopObserver(T &&t) {
        m_StopObserver = std::forward<T>(t);
    }

    ScriptApi *GetScriptApi();
    const Configuration::RuntimeConfiguration *GetConfiguration() {
        return &configuration;
    }

    // Settings::iChangeCallback
    Settings::ApplyMethod ValueChanged(const std::string &key,
                                       Settings *siface);

  private:
    InterfaceMap &interfaceMap;
    Configuration::RuntimeConfiguration configuration;

    bool m_CanWork = false;
    Memory::aligned_ptr<RenderDevice> m_Device;
    Memory::aligned_ptr<iContext> m_Context;
    Memory::aligned_ptr<Resources::ResourceManager> m_ResourceManager;
    std::unique_ptr<ScriptApi> m_ScriptApi;

    std::shared_ptr<Settings::iChangeCallback> settingsChangeCallback;

    std::function<void()> m_StopObserver;

    void ReloadConfig();
};

} // namespace MoonGlare::Renderer

#endif
