#pragma once

#include "file_table_interface.hpp"
#include "svfs/vfs_exporter.hpp"
#include "variant_argument_map.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace MoonGlare::StarVfs {

class ZipExporter : public iVfsExporter {
public:
    static constexpr const char *kClassName = "zip";

    ZipExporter(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments);
    ~ZipExporter() override;

    void StartExport() override;

private:
    std::string output_path;
    std::string root_point;
    bool include_hidden_files = false;
    bool generate_resource_id = false;
    bool deflate = false;
};

} // namespace MoonGlare::StarVfs
