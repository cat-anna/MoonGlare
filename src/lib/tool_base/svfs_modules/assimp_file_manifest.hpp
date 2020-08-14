#pragma once

#include <nlohmann/json.hpp>

namespace MoonGlare::Tools::SvfsModules {

constexpr auto kAssimpFileManifestExt = ".manifest";

struct AssimpFileManifest {
    uint32_t version{1};
};

inline void to_json(nlohmann::json &j, const AssimpFileManifest &p) {
    j = {
        {"version", p.version},
        // {"file_entries", p.file_entries},
    };
}

inline void from_json(const nlohmann::json &j, AssimpFileManifest &p) {
    j.at("version").get_to(p.version);
    // j.at("file_entries").get_to(p.file_entries);
}

} // namespace MoonGlare::Tools::SvfsModules