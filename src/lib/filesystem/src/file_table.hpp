
#pragma once

#include "file_entry.hpp"
#include <boost/pool/poolfwd.hpp>
#include <string_view>
#include <svfs/hashes.hpp>
#include <unordered_map>
#include <utility>


namespace MoonGlare::StarVfs {

class FileTable {
public:
    explicit FileTable();

    bool CreateDirectory(std::string_view path);

    FileEntry *FindFileByPath(FilePathHash path_hash) {
        auto it = file_by_path_hash.find(path_hash);
        if (it == file_by_path_hash.end()) {
            return nullptr;
        }
        return it->second;
    }
    FileEntry *FindFileByPath(const std::string_view &path) { return FindFileByPath(Hasher::Hash(path)); }

    void AddFileEntry(FileEntry *entry);

    const FileEntry *GetRootFile() const { return &root_file; }

    std::vector<std::pair<FileContentHash, FileEntry *>> GetFileContentHashMap() const;
    std::vector<std::pair<FilePathHash, FileEntry *>> GetFilePathHashMap() const;

private:
    FileEntry root_file;
    std::unordered_map<FileContentHash, FileEntry *> file_by_content_hash;
    std::unordered_map<FilePathHash, FileEntry *> file_by_path_hash;
};

} // namespace MoonGlare::StarVfs
