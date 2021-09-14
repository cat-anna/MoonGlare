#include "commands/buffer_commands.hpp"
#include "commands/frame_buffer_commands.hpp"
#include "frame_buffer.hpp"

namespace MoonGlare::Renderer {

using namespace Commands;

BaseRenderTarget::BaseRenderTarget(CommandQueue *command_queue) {
    command_queue->PushCommand(BufferBulkAllocate{
        .out = dynamic_buffer_pool.BulkInsert(dynamic_buffer_pool.Capacity()),
        .count = dynamic_buffer_pool.Capacity(),
    });

    command_queue->PushCommand(VaoBulkAllocate{
        .out = dynamic_vao_pool.BulkInsert(dynamic_vao_pool.Capacity()),
        .count = dynamic_vao_pool.Capacity(),
    });
}

void BaseRenderTarget::ReleaseResources(CommandQueue *command_queue) {
    Reset();

    command_queue->PushCommand(BufferBulkRelease{
        .out = dynamic_buffer_pool.Storage(),
        .count = dynamic_buffer_pool.ValidElements(),
    });
    dynamic_buffer_pool.InvalidateStoredElements();

    command_queue->PushCommand(VaoBulkRelease{
        .out = dynamic_vao_pool.Storage(),
        .count = dynamic_vao_pool.ValidElements(),
    });
    dynamic_vao_pool.InvalidateStoredElements();
}

math::ivec2 BaseRenderTarget::BufferSize() const {
    return frame_buffer_size;
}

void BaseRenderTarget::InitViewPortCommands() {
    command_queue.PushCommand(Commands::FrameBufferBindViewportCommand{
        .handle = frame_buffer_handle,
        .width = frame_buffer_size[0],
        .height = frame_buffer_size[1],
    });
}

} // namespace MoonGlare::Renderer