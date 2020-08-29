#pragma once

#include "component_common.hpp"
#include "math/vector.hpp"
#include "static_string.hpp"
#include <cstddef>

namespace MoonGlare::Component {

#pragma pack(push, 1)

// alignas(16)
struct Transform : public ComponentBase<Transform> {
    static constexpr ComponentId kComponentId = 4;
    static constexpr char *kComponentName = "Transform";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    // Array<RuntimeRevision>      revision;
    // Array<emath::fvec3>         globalScale;

    math::fvec3 scale{1.0f, 1.0f, 1.0f};
    bool dirty = false;
    uint8_t _padding_0[3];

    math::fvec3 position;
    float _padding_1;

    math::Quaternion quaternion;

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

#ifdef _WANTS_TYPE_INFO_

auto GetTypeInfo(Transform *) {
    return AttributeMapBuilder<Transform>::Start("Transform")
        ->AddField("scale", &Transform::scale)
        ->AddField("position", &Transform::position);
}

#endif

#ifdef _WANTS_COMPONENT_SERIALIZATION_

void to_json(nlohmann::json &j, const Transform &p) {
    j = {
        {"scale", p.scale},
        {"position", p.position},
    };
}
void from_json(const nlohmann::json &j, Transform &p) {
    j.at("scale").get_to(p.scale);
    j.at("position").get_to(p.position);
}

#endif

} // namespace MoonGlare::Component
