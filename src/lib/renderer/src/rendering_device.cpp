#include "renderer/rendering_device.hpp"
#include "frame_buffer.hpp"
#include "opengl_device_info.hpp"
#include "opengl_error_handler.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

//TODO: remove me
#include "renderer/virtual_camera.hpp"

namespace MoonGlare::Renderer {

namespace {

template <typename TIMEPOINT>
auto TimeDiff(const TIMEPOINT &start, const TIMEPOINT &end) {
    return std::chrono::duration<float>(end - start).count();
}

} // namespace

//----------------------------------------------------------------------------------

RenderingDevice::FrameQueue::FrameQueue() {
    for (auto &item : standby_frames) {
        item = nullptr;
    }
}

FrameBuffer *RenderingDevice::FrameQueue::FetchRenderPending() {
    auto r = render_pending.exchange(nullptr);
    if (r) {
        ++processed_frames;
    }
    return r;
}

FrameBuffer *RenderingDevice::FrameQueue::FetchNext() {
    for (auto &item : standby_frames) {
        auto frame = item.exchange(nullptr);
        if (frame != nullptr) {
            Drop(current_frame.exchange(frame));
            return frame;
        }
    }
    return nullptr;
}

void RenderingDevice::FrameQueue::Release(FrameBuffer *frame) {
    assert(frame);
    frame->Reset();
    for (auto &item : standby_frames) {
        frame = item.exchange(frame);
        if (frame == nullptr) {
            return;
        }
    }
}

void RenderingDevice::FrameQueue::SubmitCurrent() {
    auto current = current_frame.exchange(nullptr);
    if (!current) {
        return;
    }
    Drop(render_pending.exchange(current));
}

//----------------------------------------------------------------------------------

RenderingDevice::RenderingDevice(gsl::not_null<iEngineTime *> engine_time,
                                 gsl::not_null<iDeviceContext *> _device_context,
                                 gsl::not_null<iDeviceWindow *> _main_window,
                                 gsl::not_null<iResourceManager *> _resource_manager)
    : device_context(_device_context), main_window(_main_window),
      resource_manager(_resource_manager) {
    //TODO: init queue should not be created/executed here
    std::unique_ptr<CommandQueue> init_queue = std::make_unique<CommandQueue>();

    auto size = main_window->GetSize();

    for (auto &item : allocated_frames) {
        item = make_aligned<FrameBuffer>(engine_time, init_queue.get(), _resource_manager, size);
        frame_queue.Release(item.get());
    }

    init_queue->Execute();
}

RenderingDevice::~RenderingDevice() = default;

void RenderingDevice::EnterLoop() {
    can_work = true;

    ErrorHandler::RegisterErrorCallback();
    DeviceInfo::ReadInfo();

    auto last_time = clock_t::now();
    auto window = main_window;

    while (can_work.load()) {
        auto current_time = clock_t::now();
        if (TimeDiff(last_time, current_time) >= 1.0f) {
            last_time = current_time;
            AddLog(Performance,
                   fmt::format("Frames: processed:{:06} dropped:{:06}",
                               frame_queue.ProcessedFrames(), frame_queue.DroppedFrames()));
        }

        device_context->PoolEvents();
        ExecuteResourceTasks();
        // ProcessPendingCtrlQueues();

        auto frame_to_render = frame_queue.FetchRenderPending();
        if (frame_to_render != nullptr) {
            // using Layer = Renderer::Frame::CommandLayers::LayerEnum;

            // auto &cmdl = frame->GetCommandLayers();
            // cmdl.Get<Layer::GUI>().Sort();

            // window->BindAsRenderTarget();

            // // frame->GetCommandLayers().Execute();
            // cmdl.Execute();

            //TODO: remove me

            VirtualCamera camera;
            camera.SetUniformOrthogonal({1920.0f, 1080.0f});

            if (glIsProgram(1) == GL_TRUE) {
                glUniformMatrix4fv(glGetUniformLocation(1, "uCameraMatrix"), 1, GL_TRUE,
                                   reinterpret_cast<const float *>(&camera.projection_matrix));
            }
            if (glIsProgram(2) == GL_TRUE) {
                glUniformMatrix4fv(glGetUniformLocation(2, "uCameraMatrix"), 1, GL_TRUE,
                                   reinterpret_cast<const float *>(&camera.projection_matrix));
            }

            frame_to_render->ExecuteQueue();
            // glFlush();

            window->SwapBuffers();

            // if (captureScreenShoot.exchange(false)) {
            //     CaptureScreenshoot(Ctx);
            // }

            frame_queue.Release(frame_to_render);
        }
    }
}

void RenderingDevice::Stop() {
    can_work = false;
}

void RenderingDevice::ExecuteResourceTasks() {
    if (!resource_tasks.Empty()) {
        //TODO:
        std::unique_ptr<CommandQueue> init_queue = std::make_unique<CommandQueue>();
        CommandQueueRef ref{*init_queue};
        decltype(resource_tasks)::ContainerType tasks;
        resource_tasks.Swap(tasks);
        for (auto &task : tasks) {
            task->Execute(ref);
        }
        init_queue->Execute();
    }
}

iRenderTarget *RenderingDevice::GetDisplayRenderTarget() {
    return &render_proxy;
}

iResourceManager *RenderingDevice::GetResourceManager() {
    return resource_manager;
}

void RenderingDevice::NextFrame() {
    auto next = frame_queue.FetchNext();
    render_proxy.SetTarget(next);
}

void RenderingDevice::SubmitFrame() {
    render_proxy.SetTarget(nullptr);
    frame_queue.SubmitCurrent();
}

//----------------------------------------------------------------------------------

// void RenderingDevice::CaptureScreenshoot(MoonGlare::Renderer::iContext *Ctx) {
//     auto s = Ctx->GetSize();
//     uint32_t bytes = s[0] * s[1] * 4;
//     std::unique_ptr<uint8_t[]> memory(new uint8_t[bytes]);
//     glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//     glFlush();
//     glFinish();
//     glReadPixels(0, 0, s[0], s[1], GL_BGRA, GL_UNSIGNED_BYTE, memory.get());
//     m_RendererFacade->GetAsyncLoader()->QueueTask(
//         std::make_shared<Resources::Texture::FreeImageStore>(std::move(memory), s, 32));
//     AddLogf(Info, "Queued screen shoot store task");
// }

} // namespace MoonGlare::Renderer

#if 0

namespace MoonGlare::Renderer {

//----------------------------------------------------------------------------------

void RendererFacade::ReloadConfig() {
    auto stt = interfaceMap.GetSharedInterface<Settings>();
    assert(stt);

    //configuration.m_Texture.m_Filtering =

    configuration.ResetToDefault();

    stt->Get("Renderer.Texture.Filtering", configuration.texture.m_Filtering);

    stt->Get("Renderer.Shadow.MapSize", configuration.shadow.shadowMapSize);
    stt->Get("Renderer.Shadow.Enabled", configuration.shadow.enableShadows);

    stt->Get("Renderer.Shader.GaussianDiscLength", configuration.shader.gaussianDiscLength);
    stt->Get("Renderer.Shader.GaussianDiscRadius", configuration.shader.gaussianDiscRadius);

    stt->Get("Renderer.Gamma", configuration.gammaCorrection);
}

//----------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer

#endif