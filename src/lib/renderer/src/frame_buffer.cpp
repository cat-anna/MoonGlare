#include "frame_buffer.hpp"

namespace MoonGlare::Renderer {

FrameBuffer::FrameBuffer(gsl::not_null<iEngineTime *> engine_time,
                         gsl::not_null<CommandQueue *> command_queue,
                         gsl::not_null<iResourceManager *> _resource_manager,
                         math::ivec2 display_size)
    : BaseRenderTarget(command_queue, engine_time, _resource_manager) {
    frame_buffer_handle = Device::kInvalidFrameBufferHandle;
    frame_buffer_size = math::fvec2{display_size[0], display_size[1]};

    InitViewPortCommands();
    this->command_queue.SetPreamble();
}

} // namespace MoonGlare::Renderer