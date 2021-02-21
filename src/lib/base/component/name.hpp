#pragma once

#include "component_common.hpp"
#include "component_serialiazation.hpp"
#include "static_string.hpp"

namespace MoonGlare::Component {

struct alignas(16) Name : public ComponentBase<Name> {
    static constexpr ComponentId kComponentId = 0;
    static constexpr char kComponentName[] = "name";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = true;

    using StringType = BasicStaticString<64, char>;

    StringType value;
};

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(Name *) {
    return AttributeMapBuilder<Name>::Start(Name::kComponentName) //
        ->AddFieldWithAlias<std::string>("value", &Name::value);
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const Name &p) {
    j = {
        {"value", p.value},
    };
}
void from_json(const nlohmann::json &j, Name &p) {
    j.at("value").get_to(p.value);
}

#endif

} // namespace MoonGlare::Component
