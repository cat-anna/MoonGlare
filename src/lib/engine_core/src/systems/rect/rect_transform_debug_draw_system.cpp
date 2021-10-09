#include "rect_transform_debug_draw_system.hpp"
#include "component/global_matrix.hpp"
#include "component/local_matrix.hpp"
#include "component/rect/rect_transform.hpp"
#include "ecs/component_array.hpp"
#include "renderer/render_target_interface.hpp"
#include "renderer/resources.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Systems::Rect {

using namespace Component;
using namespace Component::Rect;

using iRenderTarget = Renderer::iRenderTarget;

RectTransformDebugDrawSystem::RectTransformDebugDrawSystem(const ECS::SystemCreateInfo &create_info,
                                                           SystemConfiguration config_data)
    : SystemBase(create_info, config_data) {
    shader_handle = GetResourceManager()->LoadShader("/shader/rect_transform_debug_draw");
    if (shader_handle == kInvalidResourceHandle) {
        AddLog(Warning, "Failed to load shader for rect transform debug draw. Deactivating system");
        SetActive(false);
    }
}

void RectTransformDebugDrawSystem::DoStep(double time_delta) {
#if 0
    // q.MakeCommandKey<Commands::Enable>(key, (GLenum)GL_BLEND);
    // q.MakeCommandKey<Commands::Disable>(key, (GLenum)GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);
#endif

    auto element_buffer = GetRenderTarget()->ReserveElements(iRenderTarget::ElementReserve{
        .index_count = last_known_element_count * 8,
        .vertex_count = last_known_element_count * 4,
    });

    if (element_buffer.index_buffer == nullptr || element_buffer.vertex_buffer == nullptr) {
        AddLogf(Warning, "Failed to allocate element buffer");
        return;
    }

    // GetRenderTarget()->AttachCamera(camera_mat);

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
                math::fvec4(0, 0, 0.0f, 1.0f),
                math::fvec4(w, 0, 0.0f, 1.0f),
                math::fvec4(w, h, 0.0f, 1.0f),
                math::fvec4(0, h, 0.0f, 1.0f),
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

    auto req = iRenderTarget::ElementRenderRequest{
        .position_matrix = math::fmat4::Identity(),
        .element_mode = GL_LINES,
        .index_count = generated_indices,
        .shader_handle = shader_handle,
    };

    GetRenderTarget()->SubmitElements(element_buffer, req);
    last_known_element_count = elements;
}

} // namespace MoonGlare::Systems::Rect
