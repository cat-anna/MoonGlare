#pragma once

#include "component_common.hpp"
#include "math/vector.hpp"
#include "static_string.hpp"
#include <cstddef>

namespace MoonGlare::Component {

#pragma pack(push, 1)

struct alignas(16) Transform : public ComponentBase<Transform> {
    static constexpr ComponentId kComponentId = 3;
    static constexpr char kComponentName[] = "transform";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    // Array<emath::fvec3>         globalScale;

    math::fvec3 scale{1.0f, 1.0f, 1.0f};
    bool dirty = false;
    uint8_t _padding_0[3];

    math::fvec3 position;
    float _padding_1;

    math::Quaternion quaternion = math::Quaternion::Identity();

    void GetLocalMatrix(math::Transform &tr) const {
        // tr.setIdentity();
        tr.rotate(quaternion);
        tr.scale(scale);
        tr.translation() = position;
    }
};

#pragma pack(pop)

static_assert((sizeof(Transform) % 16) == 0);
static_assert((offsetof(Transform, scale) % 16) == 0);
static_assert((offsetof(Transform, position) % 16) == 0);
static_assert((offsetof(Transform, quaternion) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(Transform *) {
    return AttributeMapBuilder<Transform>::Start(Transform::kComponentName)
        ->AddField("scale", &Transform::scale)
        ->AddField("position", &Transform::position)
        ->AddField("quaternion", &Transform::quaternion);
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const Transform &p) {
    j = {
        {"scale", p.scale},
        {"position", p.position},
        {"quaternion", p.quaternion},
    };
}
void from_json(const nlohmann::json &j, Transform &p) {
    j.at("scale").get_to(p.scale);
    j.at("position").get_to(p.position);
    j.at("quaternion").get_to(p.quaternion);
}

#endif

} // namespace MoonGlare::Component
