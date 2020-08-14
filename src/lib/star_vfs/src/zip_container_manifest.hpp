#include "svfs/hashes.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace MoonGlare::StarVfs {

constexpr auto kContainerManifestFileName = ".container_manifest";

struct ZipContainerManifest {
    uint32_t version = 1;

    struct FileEntry {
        FileResourceId resource_id;
    };
    std::map<std::string, FileEntry> file_entries;
};

inline void to_json(nlohmann::json &j, const ZipContainerManifest::FileEntry &p) {
    j = {{"resource_id", p.resource_id}};
}

inline void from_json(const nlohmann::json &j, ZipContainerManifest::FileEntry &p) {
    j.at("resource_id").get_to(p.resource_id);
}

inline void to_json(nlohmann::json &j, const ZipContainerManifest &p) {
    j = {{"version", p.version}, {"file_entries", p.file_entries}};
}

inline void from_json(const nlohmann::json &j, ZipContainerManifest &p) {
    j.at("version").get_to(p.version);
    j.at("file_entries").get_to(p.file_entries);
}

} // namespace MoonGlare::StarVfs