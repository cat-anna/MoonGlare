
#include "renderer/frame_sink.hpp"
#include "commands/buffer_commands.hpp"
#include "commands/shader_commands.hpp"
#include "commands/texture_commands.hpp"
#include "frame_buffer.hpp"
#include "renderer/types.hpp"

namespace MoonGlare::Renderer {

void FrameSink::SetFrameBuffer(FrameBuffer *frame_buffer) {
    current_frame_buffer = frame_buffer;
}

FrameSink::ElementBuffer FrameSink::ReserveElements(const ElementReserve &config) {
    ElementReserve reserve = config;
    RenderElementIndexType *index_buffer = nullptr;
    math::fvec3 *vertex_buffer = nullptr;
    math::fvec2 *texture0_buffer = nullptr;

    if (current_frame_buffer != nullptr) {
        auto fb = current_frame_buffer;

        if (config.index_count) {
            index_buffer = fb->AllocateElements<RenderElementIndexType>(config.index_count);
        }
        if (config.vertex_count > 0) {
            vertex_buffer = fb->AllocateElements<math::fvec3>(config.vertex_count);
        }
        if (config.texture0_count > 0) {
            texture0_buffer = fb->AllocateElements<math::fvec2>(config.texture0_count);
        }
    }

    return ElementBuffer{
        .reserve = reserve,
        .index_buffer = index_buffer,
        .vertex_buffer = vertex_buffer,
        .texture0_buffer = texture0_buffer,
    };
}

void FrameSink::SubmitElements(const ElementBuffer &buffer, const ElementRenderRequest &request) {
    auto fb = current_frame_buffer;
    if (fb == nullptr) {
        //TODO
        TriggerBreakPoint();
        return;
    }

    using namespace Commands;
    auto &queue = fb->command_queue;

    queue.PushCommand(VaoBind{.handle = fb->dynamic_vao_pool.Next()});

    using Layout = Renderer::Resources::ShaderVariables::Layout;

    auto populate_channel = [&](Layout channel, auto *data, size_t count, GLint element_count) {
        if (data != nullptr) {
            queue.PushCommand(BindArrayBuffer{.handle = fb->dynamic_buffer_pool.Next()});
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
        queue.PushCommand(BindArrayIndexBuffer{.handle = fb->dynamic_buffer_pool.Next()});
        queue.PushCommand(ArrayIndexBufferDynamicData{
            .byte_count = sizeof(RenderElementIndexType) * buffer.reserve.index_count,
            .data_ptr = buffer.index_buffer,
        });
    } else {
        //TODO:?
        TriggerBreakPoint();
    }

    queue.PushCommand(Texture2DBindCommand{
        .handle = request.texture_handle & kResourceDeviceHandleMask,
    });

    // math::fmat4 position_matrix;
    // ShaderHandle shader_handle;
    queue.PushCommand(ShaderBindCommand{
        .handle = request.shader_handle & kResourceDeviceHandleMask,
    });

    using ShaderVariables = Resources::ShaderVariables;
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

#if 0

void Frame::ReleaseResource(TextureResourceHandle &texres) {
    assert(this);
    GetResourceManager()->GetTextureResource().Release(texres);
}
void Frame::ReleaseResource(VAOResourceHandle &vaores) {
    assert(this);
    GetResourceManager()->GetVAOResource().Release(this, vaores);
}
bool Frame::AllocateResource(TextureResourceHandle &resH) {
    assert(this);
    return GetResourceManager()->GetTextureResource().Allocate(resH);
}
bool Frame::AllocateResource(VAOResourceHandle &resH) {
    assert(this);
    return GetResourceManager()->GetVAOResource().Allocate(this, resH);
}

//----------------------------------------------------------------------------------

bool Frame::Initialize(uint8_t BufferIndex, RenderDevice *device, RendererFacade *rfacade) {
    assert(BufferIndex < Configuration::FrameBuffer::Count);
    assert(device);
    assert(rfacade);

    m_BufferIndex = BufferIndex;
    m_RenderDevice = device;
    m_ResourceManager = rfacade->GetResourceManager();

    queue.PushCommand.ClearAllocation();
    m_SubQueueTable.ClearAllocation();
    //m_Textures.ClearAllocation();
    m_VAOs.ClearAllocation();
    planeShadowMaps.ClearAllocation();
    cubeShadowMaps.ClearAllocation();

    m_CommandLayers.Clear();

    m_Memory.Clear();

    for (auto &q : m_SubQueueTable)
  {      q.Clear();}

    ShadowMap psm;
    psm.textureHandle = Device::InvalidTextureHandle;
    psm.framebufferHandle = Device::InvalidFramebufferHandle;
    planeShadowMaps.fill(psm);

    configuration = rfacade->GetConfiguration();
    flags.shadowsEnabled = configuration->shadow.enableShadows;

    return true;
}

bool Frame::Finalize() {
    return true;
}

//----------------------------------------------------------------------------------

void Frame::BeginFrame(uint64_t index) {
    frameIndex = index;
    queue.PushCommand.ClearAllocation();
    m_SubQueueTable.ClearAllocation();

    m_CommandLayers.ClearAllocation();

    //m_Textures.ClearAllocation();
    m_VAOs.ClearAllocation();
    planeShadowMaps.ClearAllocation();
    cubeShadowMaps.ClearAllocation();

    m_Memory.Clear();
}

void Frame::EndFrame() {
}

//----------------------------------------------------------------------------------

bool Frame::Submit(TextureRenderTask *trt) {
    assert(trt);
    m_CommandLayers.Get<Conf::Layer::PreRender>().PushQueue(&trt->GetCommandQueue());
    return queue.PushCommand.push(trt);
}

bool Frame::Submit(SubQueue *q, Conf::Layer Layer, Commands::CommandKey Key) {
    assert(q);
    m_CommandLayers[Layer].PushQueue(q, Key);
    return false;
}

//----------------------------------------------------------------------------------

#endif
} //namespace MoonGlare::Renderer
