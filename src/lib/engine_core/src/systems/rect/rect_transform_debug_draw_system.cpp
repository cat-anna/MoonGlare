#include "rect_transform_debug_draw_system.hpp"
#include "component/global_matrix.hpp"
#include "component/local_matrix.hpp"
#include "component/rect/rect_transform.hpp"
#include "ecs/component_array.hpp"
#include "renderer/frame_sink_interface.hpp"
#include "renderer/resources.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Systems::Rect {

using namespace Component;
using namespace Component::Rect;

using iFrameSink = Renderer::iFrameSink;

RectTransformDebugDrawSystem::RectTransformDebugDrawSystem(const ECS::SystemCreateInfo &create_info,
                                                           SystemConfiguration config_data)
    : SystemBase(create_info, config_data) {

    //TODO: create dedicated shader for rect transform debug draw
    shader_handle = GetResourceManager()->LoadShader("gui");
    if (shader_handle == Renderer::kInvalidResourceHandle) {
        AddLog(Warning, "Failed to load shader for rect transform debug draw. Deactivating system");
        SetActive(false);
    }
}

void RectTransformDebugDrawSystem::DoStep(double time_delta) {
#if 0
    auto frame = conf.m_BufferFrame;

    using namespace Renderer;
    using Uniform = GUIShaderDescriptor::Uniform;


    auto &layers = frame->GetCommandLayers();
    auto &q = layers.Get<Renderer::Configuration::FrameBuffer::Layer::GUI>();
    auto &shres = frame->GetResourceManager()->GetShaderResource();
    auto key = Renderer::Commands::CommandKey::Max();
    auto shb = shres.GetBuilder<GUIShaderDescriptor>(q, shaderHandle);

    shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(glm::identity<glm::fmat4>()), key);
    shb.Set<Uniform::TileMode>(emath::ivec2(0, 0), key);
    shb.Set<Uniform::BaseColor>(emath::fvec4(1, 1, 1, 1), key);

    q.MakeCommandKey<Commands::Texture2DBind>(key, Renderer::Device::InvalidTextureHandle);
   // q.MakeCommandKey<Commands::Enable>(key, (GLenum)GL_BLEND);
    q.MakeCommandKey<Commands::Disable>(key, (GLenum)GL_CULL_FACE);
    q.MakeCommandKey<Commands::Disable>(key, (GLenum)GL_DEPTH_TEST);
    q.MakeCommandKey<Commands::EnterWireFrameMode>(key);

    // ...

    q.MakeCommandKey<Renderer::Commands::LeaveWireFrameMode>(key);
#endif

    auto element_buffer = GetFrameSink()->ReserveElements(iFrameSink::ElementReserve{
        .index_count = last_known_element_count * 8,
        .vertex_count = last_known_element_count * 4,
    });

    if (element_buffer.index_buffer == nullptr || element_buffer.vertex_buffer == nullptr) {
        AddLogf(Warning, "Failed to allocate element buffer");
        return;
    }

    size_t generated_indices = 0;
    size_t generated_vertexes = 0;

    auto index_buffer = element_buffer.index_buffer;
    auto vertex_buffer = element_buffer.vertex_buffer;

    size_t elements = 0;
    GetComponentArray()->Visit<RectTransform, GlobalMatrix>(
        [&](const RectTransform &rect, const GlobalMatrix &matrix) {
            ++elements;

            if (generated_indices >= element_buffer.reserve.index_count) {
                return;
            }

            const auto &transform = matrix.transform;
            auto w = rect.size[0];
            auto h = rect.size[1];
            std::array<math::fvec4, 4> points = {
                math::fvec4(w * 0, h * 0, 0.0f, 1.0f),
                math::fvec4(w * 1.0f, h * 0, 0.0f, 1.0f),
                math::fvec4(w * 1.0f, h * 1.0f, 0.0f, 1.0f),
                math::fvec4(w * 0, h * 1.0f, 0.0f, 1.0f),
            };

            for (uint16_t i = 0; i < points.size(); ++i) {
                auto r = transform * points[i];
                vertex_buffer[generated_vertexes + i] = math::fvec3{r[0], r[1], r[2]};
            }

            static const std::array<uint16_t, 8> indexes = {0, 1, 2, 3, 0, 3, 1, 2};
            for (uint16_t i = 0; i < indexes.size(); ++i) {
                index_buffer[generated_indices + i] = generated_vertexes + indexes[i];
            }

            generated_indices += indexes.size();
            generated_vertexes += points.size();
        });

    auto req = iFrameSink::ElementRenderRequest{
        // .position_matrix = {}, //identity
        .element_mode = (GLenum)GL_LINES, //GL_TRIANGLES,
        .index_count = (int)generated_indices,
    };

    GetFrameSink()->SubmitElements(element_buffer, req);
    last_known_element_count = elements + 16;
}

} // namespace MoonGlare::Systems::Rect
