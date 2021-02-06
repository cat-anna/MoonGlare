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
    bool WriteFileContent(FilePathHash container_file_id, const std::string &file_data) override;
    FilePathHash FindFile(const std::string &relative_path) const override;

    AccessMode GetAccessMode() const override { return access_mode; };

    struct FileEntry {
        std::filesystem::path host_path;
    };

    using FileMapper = std::unordered_map<FilePathHash, FileEntry>;
    std::string GetContainerName() const override;

private:
    AccessMode access_mode{AccessMode::ReadOnly};
    std::filesystem::path host_path;
    std::string mount_point;
    bool generate_resource_id{true};
    bool store_resource_id{false};

    FileMapper file_mapper;

    struct ScanPathOutput;
    bool ScanPath(ScanPathOutput &scan_output);

    bool CanWrite() const { return access_mode == AccessMode::ReadWrite; }
};

} // namespace MoonGlare::StarVfs
