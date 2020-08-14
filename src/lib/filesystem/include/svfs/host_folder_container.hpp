#pragma once

#include "file_table_interface.hpp"
#include "svfs/vfs_container.hpp"
#include "variant_argument_map.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace MoonGlare::StarVfs {

class HostFolderContainer : public iVfsContainer {
public:
    static constexpr const char *kClassName = "host_folder";

    HostFolderContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments);
    ~HostFolderContainer() override = default;

    void ReloadContainer() override;

    bool ReadFileContent(FilePathHash container_file_id, std::string &file_data) const override;

    struct FileEntry {
        std::filesystem::path host_path;
    };

    using FileMapper = std::unordered_map<FilePathHash, FileEntry>;

private:
    std::filesystem::path host_path;
    std::string mount_point;
    bool generate_resource_id{true};
    bool store_resource_id{false};

    FileMapper file_mapper;

    struct ScanPathOutput;
    bool HostFolderContainer::ScanPath(ScanPathOutput &scan_output);
};

} // namespace MoonGlare::StarVfs
