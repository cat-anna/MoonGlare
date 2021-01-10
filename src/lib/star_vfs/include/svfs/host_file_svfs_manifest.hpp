#pragma once

#include "svfs/hashes.hpp"
#include <json_helpers.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace MoonGlare::StarVfs {

constexpr auto kHostFileSvfsManifestExtension = ".svfs-manifest";

struct HostFileSvfsManifestSubContent {
    uint32_t version = 1;
    std::string file_name;
    FileResourceId resource_id = 0;
};

inline void to_json(nlohmann::json &j, const HostFileSvfsManifestSubContent &p) {
    j = {
        {"version", p.version},
        {"file_name", p.file_name},
        {"resource_id", p.resource_id},
    };
}

inline void from_json(const nlohmann::json &j, HostFileSvfsManifestSubContent &p) {
    j.at("version").get_to(p.version);
    j.at("file_name").get_to(p.file_name);
    j.at("resource_id").get_to(p.resource_id);
}

struct HostFileSvfsManifest {
    uint32_t version = 1;
    FileResourceId resource_id = 0;
    bool hide_original_file = false;
    XXH64_hash_t content_hash = 0;
    std::vector<HostFileSvfsManifestSubContent> sub_contents;
    std::string content_imported_by;
};

inline void to_json(nlohmann::json &j, const HostFileSvfsManifest &p) {
    j = {
        {"version", p.version},
        {"resource_id", p.resource_id},
        {"sub_contents", p.sub_contents}, //
        {"hide_original_file", p.hide_original_file},
        {"content_hash", p.content_hash},
        {"content_imported_by", p.content_imported_by},
    };
}
inline void from_json(const nlohmann::json &j, HostFileSvfsManifest &p) {
    j.at("resource_id").get_to(p.resource_id);
    j.at("version").get_to(p.version);
    j.at("sub_contents").get_to(p.sub_contents);
    try_get_json_child(j, "hide_original_file", p.hide_original_file, false);
    try_get_json_child(j, "content_hash", p.content_hash, 0);
    try_get_json_child(j, "content_imported_by", p.content_imported_by, "");
}

} // namespace MoonGlare::StarVfs
