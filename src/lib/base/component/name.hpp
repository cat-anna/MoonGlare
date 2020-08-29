#pragma once

#include "component_common.hpp"
#include "static_string.hpp"

namespace MoonGlare::Component {

struct Name : public ComponentBase<Name> {
    static constexpr ComponentId kComponentId = 0;
    static constexpr char *kComponentName = "Name";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = true;

    using StringType = BasicStaticString<64, char>;

    StringType value;
};

#ifdef _WANTS_TYPE_INFO_

auto GetTypeInfo(Name *) {
    return AttributeMapBuilder<Name>::Start("Name") //
        ->AddFieldWithAlias<std::string>("value", &Name::value);
}

#endif

#ifdef _WANTS_COMPONENT_SERIALIZATION_

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
