#pragma once

#include "../component_common.hpp"
#include "math/vector.hpp"
#include "rect_enums.hpp"
#include <cstddef>

namespace MoonGlare::Component::Rect {

struct
    //alignas(16)
    Image : public ComponentBase<Image> {
    static constexpr ComponentId kComponentId = 17;
    static constexpr char kComponentName[] = "image";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    // Array<emath::fvec3>         globalScale;

    // math::fvec3 scale{1.0f, 1.0f, 1.0f};
    // bool dirty = false;
    // uint8_t _padding_0[3];

    // math::fvec3 position;
    // float _padding_1;

    // math::Quaternion quaternion = math::Quaternion::Identity();

    // void GetLocalMatrix(math::Transform &tr) const {
    //     // tr.setIdentity();
    //     tr.rotate(quaternion);
    //     tr.scale(scale);
    //     tr.translation() = position;
    // }
};

// static_assert((sizeof(Image) % 16) == 0);
// static_assert((offsetof(Image, scale) % 16) == 0);
// static_assert((offsetof(Image, position) % 16) == 0);
// static_assert((offsetof(Image, quaternion) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(Image *) {
    return AttributeMapBuilder<Image>::Start(Image::kComponentName)
        // ->AddField("scale", &Image::scale)
        // ->AddField("position", &Image::position)
        // ->AddField("quaternion", &Image::quaternion)]
        ;
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const Image &p) {
    j = {
        // {"scale", p.scale},
        // {"position", p.position},
        // {"quaternion", p.quaternion},
    };
}
void from_json(const nlohmann::json &j, Image &p) {
    // j.at("scale").get_to(p.scale);
    // j.at("position").get_to(p.position);
    // j.at("quaternion").get_to(p.quaternion);
}

#endif

} // namespace MoonGlare::Component::Rect
