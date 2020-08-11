#include "file_table.hpp"
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::StarVfs {

FileTable::FileTable() {
    // allocate root file
    root_file.file_path_hash = Hasher::Hash(std::string_view(""));
    AddFileEntry(&root_file);
}

bool FileTable::CreateDirectory(std::string_view path) {
    if (path.size() < 1 || path[0] != '/') {
        return false;
    }

    if (FindFileByPath(path) != nullptr) {
        return true;
    }

    std::vector<std::string> path_parts;
    path.remove_prefix(1);
    if (path.empty()) {
        return true;
    }

    boost::split(path_parts, path, boost::is_any_of("/"));

    std::string parent_position = "";
    for (auto &item : path_parts) {
        std::string position = parent_position + "/" + item;

        auto relative_hash = Hasher::Hash(position);
        auto child = FindFileByPath(relative_hash);
        if (child == nullptr) {
            auto parent = FindFileByPath(parent_position);
            if (!parent) {
                AddLogf(Error, "Failed to create directory '%s' parent does not exists", position.c_str());
                return false;
            }

            child = parent->AddChild(item, relative_hash);
            AddFileEntry(child);
        }
    }

    return true;
}

void FileTable::AddFileEntry(FileEntry *entry) { //
    file_by_path_hash.insert_or_assign(entry->file_path_hash, entry);
    if (entry->file_content_hash != 0) {
        file_by_content_hash.insert_or_assign(entry->file_content_hash, entry);
    }
}

std::vector<std::pair<FileContentHash, FileEntry *>> FileTable::GetFileContentHashMap() const {
    std::vector<std::pair<FileContentHash, FileEntry *>> r;
    r.reserve(file_by_content_hash.size());
    for (auto &item : file_by_content_hash)
        r.emplace_back(item);
    return r;
}

std::vector<std::pair<FilePathHash, FileEntry *>> FileTable::GetFilePathHashMap() const {
    std::vector<std::pair<FilePathHash, FileEntry *>> r;
    r.reserve(file_by_path_hash.size());
    for (auto &item : file_by_path_hash)
        r.emplace_back(item);
    return r;
}

} // namespace MoonGlare::StarVfs
