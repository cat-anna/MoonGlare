#pragma once

#include "component_common.hpp"
#include "component_serialiazation.hpp"
#include "math/camera_projection.hpp"
#include "math/vector.hpp"

namespace MoonGlare::Component {

struct Camera : public ComponentBase<Camera> {
    static constexpr ComponentId kComponentId = 7;
    static constexpr char kComponentName[] = "camera";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;
    static constexpr bool kHasResources = false;

    math::fmat4 projection_matrix = math::fmat4::Identity();
    math::fvec3 up_vector = math::fvec3{0, 1, 0};
    // math::fvec3 up_vector = math::fvec3{0, 0, 1};
    bool orthogonal;
    float fov;
    //  float Near = 0.1f, Far = 1.0e4f;
    float near = 0.1f;
    float far = 1.0e4f;

    void ResetProjectionMatrix(float aspect) {
        if (orthogonal) {
            projection_matrix = math::Ortho(-aspect, aspect, 1.0f, -1.0f);
        } else {
            projection_matrix = math::Perspective(fov, aspect, near, far);
        }
    }
};

// static_assert((sizeof(Camera) % 16) == 0);
static_assert((offsetof(Camera, projection_matrix) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(Camera *) {
    return AttributeMapBuilder<Camera>::Start(Camera::kComponentName)
        ->AddField("orthogonal", &Camera::orthogonal)
        ->AddField("fov", &Camera::fov)
        ->AddField("near", &Camera::near)
        ->AddField("far", &Camera::far);
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const Camera &p) {
    j = {
        {"orthogonal", p.orthogonal},
        {"fov", p.fov},
        {"near", p.near},
        {"far", p.far},
    };
}
void from_json(const nlohmann::json &j, Camera &p) {
    j.at("orthogonal").get_to(p.orthogonal);
    j.at("fov").get_to(p.fov);
    // TODO;
    // p.near = 1.0f;
    // p.far = 1.0e4f;
}

#endif

} // namespace MoonGlare::Component
