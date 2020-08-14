#pragma once

#include "file_table_interface.hpp"
#include "svfs/vfs_exporter.hpp"
#include "variant_argument_map.hpp"

namespace MoonGlare::StarVfs {

class ContentInfoExporter : public iVfsExporter {
public:
    static constexpr const char *kClassName = "content_info";

    ContentInfoExporter(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments);
    ~ContentInfoExporter() override;

    void StartExport() override;

private:
    std::string output_path;
    std::string root_point;
    bool include_hidden_files = false;
};

} // namespace MoonGlare::StarVfs
