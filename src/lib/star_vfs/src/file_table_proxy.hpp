
#pragma once

#include "file_table.hpp"
#include "svfs/file_table_interface.hpp"

namespace MoonGlare::StarVfs {

class FileTableProxy : public iFileTableInterface {
public:
    explicit FileTableProxy(uint32_t container_id, FileTable *const file_table)
        : source_container_id(container_id), file_table(file_table) {}
    ~FileTableProxy() override = default;

    bool RegisterFileStructure(const std::vector<ContainerFileEntry> &Structure) override;
    void CreateDirectory(const std::string_view &path) override;

private:
    uint32_t source_container_id;
    FileTable *const file_table;
};

} // namespace MoonGlare::StarVfs
