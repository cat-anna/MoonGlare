
#pragma once

#include "component_serialiazation.hpp"
#include "json_helpers.hpp"
#include "resource_id.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>

namespace MoonGlare::Component {

struct JsonEntity {
    bool enabled = true;
    std::optional<FileResourceId> import_id = std::nullopt;
    const nlohmann::json *components = nullptr;
    const nlohmann::json *children = nullptr;
};

// void to_json(nlohmann::json &j, const JsonEntity &p) {
//     j = {
//         {"active", p.active},
//     };
// }

void from_json(const nlohmann::json &j, JsonEntity &p) {
    p = JsonEntity{};

    try_get_json_child_default(j, "enabled", p.enabled, true, false);
    try_get_json_optional_child(j, "import", p.import_id, false);

    if (j.contains("children")) {
        p.children = &j["children"];
    }
    if (j.contains("components")) {
        p.components = &j["components"];
    }
}

//----------------------------------------------------------------------------------

struct JsonComponent {
    bool enabled = true;
    bool active = true;

    std::string class_name;
    ComponentId component_id; //= kInvalidComponentId;

    const nlohmann::json *data = nullptr;
};

// void to_json(nlohmann::json &j, const JsonComponent &p) {
//     j = {
//         {"active", p.active},
//     };
// }

void from_json(const nlohmann::json &j, JsonComponent &p) {
    p = JsonComponent{};

    try_get_json_child_default(j, "enabled", p.enabled, true, false);
    try_get_json_child_default(j, "active", p.active, true, false);
    try_get_json_child_default(j, "class", p.class_name, std::string("?"), false);
    j.at("component_id").get_to(p.component_id);

    if (j.contains("data")) {
        p.data = &j["data"];
    }
}

} // namespace MoonGlare::Component
