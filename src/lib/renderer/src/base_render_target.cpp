#include "commands/buffer_commands.hpp"
#include "commands/frame_buffer_commands.hpp"
#include "commands/shader_commands.hpp"
#include "commands/texture_commands.hpp"
#include "frame_buffer.hpp"
#include "renderer/types.hpp"

namespace MoonGlare::Renderer {

namespace {

using ShaderVariables = Resources::ShaderVariables;

template <typename T>
void SetShaderUniform(CommandQueue &q, const T &value, Device::ShaderUniformHandle uniform) {
    if (uniform != Device::kInvalidShaderUniformHandle) {
        q.PushCommand(
            Commands::ShaderSetUniformCommand<T>{.value = value, .uniform_handle = uniform});
    }
}

template <typename T>
void SetShaderUniform(CommandQueue &q, const T &value, ShaderVariables sv,
                      ShaderVariables::Uniform uniform) {
    SetShaderUniform(q, value, sv->GetUniform(uniform));
}

} // namespace

using namespace Commands;

BaseRenderTarget::BaseRenderTarget(gsl::not_null<CommandQueue *> command_queue,
                                   gsl::not_null<iEngineTime *> engine_time,
                                   gsl::not_null<iResourceManager *> res_manager)
    : engine_time(engine_time), resource_manager(res_manager) {
    command_queue->PushCommand(BufferBulkAllocate{
        .out = dynamic_buffer_pool.BulkInsert(dynamic_buffer_pool.Capacity()),
        .count = dynamic_buffer_pool.Capacity(),
    });

    command_queue->PushCommand(VaoBulkAllocate{
        .out = dynamic_vao_pool.BulkInsert(dynamic_vao_pool.Capacity()),
        .count = dynamic_vao_pool.Capacity(),
    });
}

void BaseRenderTarget::ReleaseResources(CommandQueueRef &command_queue) {
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

math::fvec2 BaseRenderTarget::BufferSize() const {
    return frame_buffer_size;
}

void BaseRenderTarget::InitViewPortCommands() {
    command_queue.PushCommand(Commands::FrameBufferBindViewportCommand{
        .handle = frame_buffer_handle,
        .width = static_cast<int>(frame_buffer_size[0]),
        .height = static_cast<int>(frame_buffer_size[1]),
    });
}

iRenderTarget::ElementBuffer BaseRenderTarget::ReserveElements(const ElementReserve &config) {
    ElementReserve reserve = config;
    RenderElementIndexType *index_buffer = nullptr;
    math::fvec3 *vertex_buffer = nullptr;
    math::fvec2 *texture0_buffer = nullptr;

    if (config.index_count) {
        index_buffer = AllocElements<RenderElementIndexType>(config.index_count);
    }
    if (config.vertex_count > 0) {
        vertex_buffer = AllocElements<math::fvec3>(config.vertex_count);
    }
    if (config.texture0_count > 0) {
        texture0_buffer = AllocElements<math::fvec2>(config.texture0_count);
    }

    return ElementBuffer{
        .reserve = reserve,
        .index_buffer = index_buffer,
        .vertex_buffer = vertex_buffer,
        .texture0_buffer = texture0_buffer,
    };
}

void BaseRenderTarget::SubmitElements(const ElementBuffer &buffer,
                                      const ElementRenderRequest &request) {
    using namespace Commands;
    auto &queue = command_queue;

    queue.PushCommand(VaoBind{.handle = dynamic_vao_pool.Next()});

    using Layout = Renderer::Resources::ShaderVariables::Layout;

    auto populate_channel = [&](Layout channel, auto *data, size_t count, GLint element_count) {
        if (data != nullptr) {
            queue.PushCommand(BindArrayBuffer{.handle = dynamic_buffer_pool.Next()});
            queue.PushCommand(ArrayBufferDynamicData{
                .byte_count = sizeof(*data) * element_count * count,
                .data_ptr = data,
            });
            queue.PushCommand(ArrayBufferChannel{
                .channel = static_cast<GLuint>(channel),
                .element_size = element_count,
                .element_type = TypeId<decltype(*data)>,
            });
        } else {
            queue.PushCommand(ArrayBufferDisableChannel{
                .channel = static_cast<GLuint>(channel),
            });
        }
    };

    populate_channel(Layout::kVertexPosition, reinterpret_cast<float *>(buffer.vertex_buffer),
                     buffer.reserve.vertex_count, 3);
    populate_channel(Layout::kTexture0UV, reinterpret_cast<float *>(buffer.texture0_buffer),
                     buffer.reserve.texture0_count, 2);

    // vaob.CreateChannel(ichannels::Texture0);
    // vaob.SetChannelData<float, 2>(ichannels::Texture0, nullptr, 0, true);

    if (buffer.index_buffer != nullptr) {
        queue.PushCommand(BindArrayIndexBuffer{.handle = dynamic_buffer_pool.Next()});
        queue.PushCommand(ArrayIndexBufferDynamicData{
            .byte_count = sizeof(RenderElementIndexType) * buffer.reserve.index_count,
            .data_ptr = buffer.index_buffer,
        });
    } else {
        //TODO:?
        TriggerBreakPoint();
    }

    queue.PushCommand(
        Texture2DBindCommand{.handle = request.texture_handle & kResourceDeviceHandleMask});

    // math::fmat4 position_matrix;
    // ShaderHandle shader_handle;
    AttachShader(request.shader_handle);
    queue.PushCommand(
        ShaderBindCommand{.handle = request.shader_handle & kResourceDeviceHandleMask});

    auto uniforms = resource_manager->GetShaderVariables(request.shader_handle);
    queue.PushCommand(ShaderSetUniformCommand<math::fmat4>{
        .value = request.position_matrix,
        .uniform_handle = uniforms->GetUniform(ShaderVariables::Uniform::kModelMatrix),
    });

    // q.MakeCommandKey<Renderer::Commands::VAODrawArrays>(key, (GLenum)GL_QUADS, (GLint)0, indexindex);
    queue.PushCommand(VaoDrawElements{
        .mode = request.element_mode,
        .count = static_cast<GLsizei>(request.index_count),
        .index_value_type = TypeId<RenderElementIndexType>,
    });

    queue.PushCommand(Commands::VaoRelease{});
}

void BaseRenderTarget::AttachShader(ShaderHandle shader_handle) {
    auto id = shader_handle & kResourceDeviceHandleMask;
    if (attached_shaders[id]) {
        return;
    }
    attached_shaders[id] = true;

    using ShaderVariables = Resources::ShaderVariables;
    auto uniforms = resource_manager->GetShaderVariables(shader_handle);

    auto &queue = command_queue;

    //TODO: optimize out virtual calls when shader uniform is not used

    SetShaderUniform(queue, frame_buffer_size,
                     uniforms->GetUniform(ShaderVariables::Uniform::kViewportSize));
    SetShaderUniform(queue, static_cast<float>(engine_time->GetGlobalTime()),
                     uniforms->GetUniform(ShaderVariables::Uniform::kGlobalTime));
    SetShaderUniform(queue, static_cast<float>(engine_time->GetCurrentTimeDelta()),
                     uniforms->GetUniform(ShaderVariables::Uniform::kTimeDelta));
}

} // namespace MoonGlare::Renderer