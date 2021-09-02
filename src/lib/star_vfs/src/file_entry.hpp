#pragma once

#include <fmt/format.h>
#include <string>
#include <string_view>
#include <svfs/hashes.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

class FileEntry {
public:
    std::string file_name; // TODO: switch to memory pool

    ContainerFileId container_file_id{0};
    uint32_t container_id{0};

    FilePathHash parent_path_hash{0};
    FilePathHash file_path_hash{0};
    FileResourceId resource_id{0};

    FileEntry *parent{nullptr};
    std::vector<std::unique_ptr<FileEntry>> children; // TODO:

    std::string GetFullPath() const;
    FileEntry *AddChild(std::string child_file_name, FilePathHash child_path_hash);

    mutable bool is_hidden = false; // TODO

    bool IsDirectory() const { return !children.empty(); }
    bool IsHidden() const { return (file_name.size() > 0 && file_name[0] == '.') || is_hidden; }

    static std::string StringHeader() {
        return "parent_path_hash | file_path_hash   | resource_id      | cid | "
               "c_file_id        | children | HD | name";
    }

    std::string String() const {
        return fmt::format("{:016x} | {:016x} | {:016x} | {:03x} | {:016x} | {:04}     | {}{} | {}",
                           parent_path_hash, file_path_hash, resource_id, container_id,
                           container_file_id, children.size(), (IsHidden() ? "H" : " "),
                           (IsDirectory() ? "D" : " "), GetFullPath());
    }
};

} // namespace MoonGlare::StarVfs
