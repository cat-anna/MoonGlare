#include "frame_buffer.hpp"
#include "commands/buffer_commands.hpp"

namespace MoonGlare::Renderer {

using namespace Commands;

FrameBuffer::FrameBuffer(CommandQueue *command_queue) {
    command_queue->PushCommand(BufferBulkAllocate{
        .out = dynamic_buffer_pool.BulkInsert(dynamic_buffer_pool.Capacity()),
        .count = dynamic_buffer_pool.Capacity(),
    });

    command_queue->PushCommand(VaoBulkAllocate{
        .out = dynamic_vao_pool.BulkInsert(dynamic_vao_pool.Capacity()),
        .count = dynamic_vao_pool.Capacity(),
    });
}

void FrameBuffer::ReleaseResources(CommandQueue *command_queue) {
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

} // namespace MoonGlare::Renderer