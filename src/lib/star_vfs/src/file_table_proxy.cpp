#include "file_table_proxy.hpp"
#include <orbit_logger.h>

namespace MoonGlare::StarVfs {

bool FileTableProxy::RegisterFileStructure(const std::vector<ContainerFileEntry> &structure) {
    for (auto &item : structure) {
        auto *child = file_table->FindFileByPath(item.file_path_hash);

        if (child == nullptr) {
            auto *parent = file_table->FindFileByPath(item.parent_path_hash);
            if (!parent) {
                AddLogf(Error, "Cannot find parent for '%s'", item.file_name.c_str());
                return false;
            }

            child = parent->AddChild(item.file_name, item.file_path_hash);

            if (!item.is_directory) {
                child->container_id = source_container_id;
                child->container_file_id = item.container_file_id;
            }

            child->resource_id = item.resource_id;
            file_table->AddFileEntry(child);
        } else {
            if (child->IsDirectory() != item.is_directory) {
                AddLogf(Error, "Possible file path hash collision. CHECKS needs improvements to be sure. New file "
                               "entry is ignored");
            }
        }
    }

    return true;
}

void FileTableProxy::CreateDirectory(const std::string_view &path) { file_table->CreateDirectory(path); }

} // namespace MoonGlare::StarVfs
