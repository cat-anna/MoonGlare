#pragma once

#include <string>
#include <string_view>
#include <svfs/hashes.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

struct FileEntry {
    std::string file_name; // TODO: switch to memory pool

    ContainerFileId container_file_id{0};
    uint32_t container_id{0};

    FilePathHash parent_path_hash{0};
    FilePathHash file_path_hash{0};
    FileContentHash file_content_hash{0};

    FileEntry *parent{nullptr};
    std::vector<std::unique_ptr<FileEntry>> children; // TODO:

    std::string GetFullPath() const;
    FileEntry *AddChild(std::string child_file_name, FilePathHash child_path_hash);

    bool IsDirectory() const { return !children.empty(); }
};

} // namespace MoonGlare::StarVfs
