#pragma once

#include "../component_common.hpp"
#include "math/vector.hpp"
#include "rect_enums.hpp"
#include <cstddef>

namespace MoonGlare::Component::Rect {

// Margin order: Left Right Top Bottom

using Margin = math::fvec4;
using Point = math::fvec2;
using Rect = math::fvec4;

static constexpr size_t kMarginIndexLeft = 0;
static constexpr size_t kMarginIndexRight = 1;
static constexpr size_t kMarginIndexTop = 2;
static constexpr size_t kMarginIndexBottom = 3;

inline float Horizontal(const Margin &margin) {
    return margin[kMarginIndexLeft] + margin[kMarginIndexTop];
}
inline float Vertical(const Margin &margin) {
    return margin[kMarginIndexTop] + margin[kMarginIndexBottom];
}
inline Point LeftTop(const Margin &margin) {
    return Point(margin[kMarginIndexLeft], margin[kMarginIndexTop]);
}
inline Point RightBottom(const Margin &margin) {
    return Point(margin[kMarginIndexRight], margin[kMarginIndexBottom]);
}
inline Point TotalMargin(const Margin &margin) {
    return Point(Horizontal(margin), Vertical(margin));
}

inline Point GetRectSize(const Rect &rect) {
    return LeftTop(rect) - RightBottom(rect);
}

struct alignas(16) RectTransform : public ComponentBase<RectTransform> {
    static constexpr ComponentId kComponentId = 16;
    static constexpr char kComponentName[] = "rect.transform";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;
    static constexpr bool kHasResources = false;

    ComponentRevision revision;

    AlignMode align_mode;

    math::fvec3 position;
    math::fvec3 size;
    // math::fvec3 scale;
    Margin margin;

    void SetDirty() { revision = 0; }
};

static_assert((sizeof(RectTransform) % 16) == 0);
// static_assert((offsetof(RectTransform, scale) % 16) == 0);
// static_assert((offsetof(RectTransform, position) % 16) == 0);
// static_assert((offsetof(RectTransform, quaternion) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(RectTransform *) {
    return AttributeMapBuilder<RectTransform>::Start(RectTransform::kComponentName)
        ->AddField("align_mode", &RectTransform::align_mode)
        ->AddField("position", &RectTransform::position)
        ->AddField("size", &RectTransform::size)
        // ->AddField("scale", &RectTransform::scale)
        ->AddField("margin", &RectTransform::margin)
        //
        ;
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const RectTransform &p) {
    j = {
        {"align_mode", p.align_mode},
        {"position", p.position},
        {"size", p.size},
        // {"scale", p.scale},
        {"margin", p.margin},
    };
}
void from_json(const nlohmann::json &j, RectTransform &p) {
    j.at("align_mode").get_to(p.align_mode);
    j.at("position").get_to(p.position);
    j.at("size").get_to(p.size);
    // j.at("scale").get_to(p.scale);
    j.at("margin").get_to(p.margin);
    p.SetDirty();
}

#endif

} // namespace MoonGlare::Component::Rect
