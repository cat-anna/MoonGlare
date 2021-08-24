
#include "ecs/system_interface.hpp"
#include "json_helpers.hpp"

namespace MoonGlare::ECS {

void to_json(nlohmann::json &j, const BaseSystemConfig &p) {
    j = {
        {"active", p.active},
    };
}

void from_json(const nlohmann::json &j, BaseSystemConfig &p) {
    try_get_json_child_default(j, "active", p.active, true);
}

} // namespace MoonGlare::ECS
