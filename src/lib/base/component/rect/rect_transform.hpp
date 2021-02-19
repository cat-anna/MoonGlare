#pragma once

#include "../component_common.hpp"
#include "math/vector.hpp"
#include "rect_enums.hpp"
#include <cstddef>

namespace MoonGlare::Component::Rect {

struct Point {};
struct Margin {};

struct alignas(16) RectTransform : public ComponentBase<RectTransform> {
    static constexpr ComponentId kComponentId = 16;
    static constexpr char kComponentName[] = "rect_transform";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    // RectTransformComponentEntryFlagsMap m_Flags;

    AlignMode align_mode;
    uint16_t z;

    Point position;
    Point size;
    //TODO: margin property

    Margin margin;

    // math::mat4 m_GlobalMatrix;
    // math::mat4 m_LocalMatrix;
    // Rect m_ScreenRect;

    // MoonGlare::Configuration::RuntimeRevision m_Revision;

    // void Recalculate(RectTransformComponentEntry &Parent);

    // void SetDirty() {
    //     m_Revision = 0;
    //     m_Flags.m_Map.m_Dirty = true;
    // }
    // void Reset() {
    //     m_Revision = 0;
    //     m_Flags.ClearAll();
    // }
};

static_assert((sizeof(RectTransform) % 16) == 0);
// static_assert((offsetof(RectTransform, scale) % 16) == 0);
// static_assert((offsetof(RectTransform, position) % 16) == 0);
// static_assert((offsetof(RectTransform, quaternion) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(RectTransform *) {
    return AttributeMapBuilder<RectTransform>::Start(RectTransform::kComponentName)
        // ->AddField("scale", &RectTransform::scale)
        // ->AddField("position", &RectTransform::position)
        // ->AddField("quaternion", &RectTransform::quaternion)]
        ;
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const RectTransform &p) {
    j = {
        // {"scale", p.scale},
        // {"position", p.position},
        // {"quaternion", p.quaternion},
    };
}
void from_json(const nlohmann::json &j, RectTransform &p) {
    // j.at("scale").get_to(p.scale);
    // j.at("position").get_to(p.position);
    // j.at("quaternion").get_to(p.quaternion);
}

#endif

} // namespace MoonGlare::Component::Rect
