#include "svfs/hashes.hpp"
#include <json_helpers.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace MoonGlare::StarVfs {

constexpr auto kHostFileSvfsManifestExtension = ".svfs-manifest";

struct HostFileSvfsManifestSubContent {
    uint32_t version = 1;
    FileResourceId resource_id;
};

void to_json(nlohmann::json &j, const HostFileSvfsManifestSubContent &p) {
    j = {
        {"version", p.version},
        {"resource_id", p.resource_id},
    };
}
void from_json(const nlohmann::json &j, HostFileSvfsManifestSubContent &p) {
    j.at("resource_id").get_to(p.resource_id);
    j.at("version").get_to(p.version);
}

struct HostFileSvfsManifest {
    uint32_t version = 1;
    FileResourceId resource_id;
    std::unordered_map<std::string, HostFileSvfsManifestSubContent> sub_contents;
};

void to_json(nlohmann::json &j, const HostFileSvfsManifest &p) {
    j = {
        {"version", p.version},
        {"resource_id", p.resource_id},
        {"sub_contents", p.sub_contents},
    };
}
void from_json(const nlohmann::json &j, HostFileSvfsManifest &p) {
    j.at("resource_id").get_to(p.resource_id);
    j.at("version").get_to(p.version);
    j.at("sub_contents").get_to(p.sub_contents);
}

} // namespace MoonGlare::StarVfs
