#include "frame_buffer.hpp"

namespace MoonGlare::Renderer {

FrameBuffer::FrameBuffer(CommandQueue *command_queue, math::ivec2 display_size)
    : BaseRenderTarget(command_queue) {
    frame_buffer_handle = Device::kInvalidFrameBufferHandle;
    frame_buffer_size = display_size;

    InitViewPortCommands();
    this->command_queue.SetPreamble();
}

} // namespace MoonGlare::Renderer