#include "file_table.hpp"
#include "svfs/path_utils.hpp"
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::StarVfs {

FileTable::FileTable() {
    // allocate root file
    root_file.file_path_hash = Hasher::Hash(std::string_view("/"));
    AddFileEntry(&root_file);
}

bool FileTable::CreateDirectory(const std::string_view &path) {
    if (path.size() < 1) {
        return false;
    }

    if (FindFileByPath(path) != nullptr) {
        return true;
    }

    std::vector<std::string> path_parts;
    if (path.empty()) {
        return true;
    }

    boost::split(path_parts, path, boost::is_any_of("/"));

    std::string parent_position = "";
    for (auto &item : path_parts) {
        if (item.empty())
            continue;
        std::string position = JoinPath(parent_position, item);

        auto relative_hash = Hasher::Hash(position);
        auto child = FindFileByPath(relative_hash);
        if (child == nullptr) {
            auto parent = FindFileByPath(parent_position);
            if (!parent) {
                AddLogf(Error, "Failed to create directory '%s' parent does not exists",
                        position.c_str());
                return false;
            }

            child = parent->AddChild(item, relative_hash);
            AddFileEntry(child);
        }
        parent_position = std::move(position);
    }

    return true;
}

void FileTable::AddFileEntry(FileEntry *entry) {
    file_by_path_hash.insert_or_assign(entry->file_path_hash, entry);
    if (entry->resource_id != 0) {
        file_by_resource_id.insert_or_assign(entry->resource_id, entry);
    }
}

std::vector<std::pair<FileResourceId, FileEntry *>> FileTable::GetResourceIdMap() const {
    std::vector<std::pair<FileResourceId, FileEntry *>> r;
    r.reserve(file_by_resource_id.size());
    for (auto &item : file_by_resource_id)
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
