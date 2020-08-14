#include "file_entry.hpp"
#include "svfs/hashes.hpp"
#include "svfs/path_utils.hpp"
#include <sstream>

namespace MoonGlare::StarVfs {

std::string FileEntry::GetFullPath() const {
    if (parent == nullptr)
        return "";
    return JoinPath(parent->GetFullPath(), file_name);
}

FileEntry *FileEntry::AddChild(std::string child_file_name, FilePathHash child_path_hash) {
    std::unique_ptr<FileEntry> entry = std::make_unique<FileEntry>();
    entry->file_name = std::move(child_file_name);
    entry->parent = this;
    entry->parent_path_hash = file_path_hash;
    entry->file_path_hash = child_path_hash;
    children.emplace_back(std::move(entry));
    return children.back().get();
}

} // namespace MoonGlare::StarVfs
