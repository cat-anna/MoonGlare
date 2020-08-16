#pragma once

#include "unique_table.h"
#include <cstdint>
#include <string>
#include <svfs/hashes.hpp>
#include <vector>

namespace MoonGlare {

using ByteTable = StarVfs::unique_table<uint8_t>;

struct FileInfo {
    std::string_view file_name;
    bool is_directory;
    bool is_hidden;
    StarVfs::FilePathHash file_path_hash;
    StarVfs::FilePathHash parent_path_hash;
};
using FileInfoTable = std::vector<FileInfo>;

} // namespace MoonGlare
