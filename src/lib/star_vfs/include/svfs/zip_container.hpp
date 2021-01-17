#pragma once

#include "svfs/file_table_interface.hpp"
#include "svfs/hashes.hpp"
#include "svfs/vfs_container.hpp"
#include <filesystem>
#include <unordered_map>
#include <variant_argument_map.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

class ZipContainer : public iVfsContainer {
public:
    static constexpr const char *kClassName = "zip";

    ZipContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments);
    ~ZipContainer() override = default;

    void ReloadContainer() override;

    bool ReadFileContent(FilePathHash container_file_id, std::string &file_data) const override;
    FilePathHash FindFile(const std::string &relative_path) const override;
    std::string GetContainerName() const override;

private:
    std::filesystem::path zip_file_path;
    std::string zip_password;
    std::string mount_point;

    struct ZipMapper;
    std::unique_ptr<ZipMapper> zip_mapper;
};

} // namespace MoonGlare::StarVfs
