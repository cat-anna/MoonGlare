#pragma once

#include "svfs/file_table_interface.hpp"
#include "svfs/hashes.hpp"
#include "svfs/host_file_svfs_manifest.hpp"
#include "svfs/vfs_container.hpp"
#include <filesystem>
#include <runtime_modules.h>
#include <unordered_map>
#include <variant_argument_map.hpp>
#include <vector>

namespace MoonGlare::Tools::SvfsModules {

class AssimpContainer : public StarVfs::iVfsContainer {
public:
    static constexpr const char *kClassName = "assimp_file";

    AssimpContainer(StarVfs::iFileTableInterface *fti, const VariantArgumentMap &arguments);
    ~AssimpContainer() override = default;

    void ReloadContainer() override;

    bool ReadFileContent(StarVfs::FilePathHash container_file_id, std::string &file_data) const override;
    std::string GetContainerName() const override;

    struct FileInfo {
        std::string file_name;
        std::string file_data;

        StarVfs::FileResourceId resource_id{0};
        StarVfs::FilePathHash container_id{0};
        StarVfs::FilePathHash path_id{0};
        StarVfs::FilePathHash parent_id{0};
    };

    void LoadFromMemory(StarVfs::FileResourceId source_resource_id, const std::string_view &file_data,
                        StarVfs::HostFileSvfsManifest &file_manifest, const std::string_view &ext);
    void LoadFromCache(StarVfs::FileResourceId source_resource_id, const std::string_view &file_data,
                       StarVfs::HostFileSvfsManifest &file_manifest, const std::string_view &ext);

private:
    std::string mount_point;
    std::string file_data;
    StarVfs::FileResourceId source_resource_id;

    std::unordered_map<StarVfs::FilePathHash, std::unique_ptr<FileInfo>> loaded_files;
    SharedModuleManager module_manager;
};

} // namespace MoonGlare::Tools::SvfsModules
