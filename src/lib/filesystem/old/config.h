#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace StarVFS {

struct Settings {
    struct Initial {
        enum {
            FileTableSize = 1 * 1024,
            StringTableSize = 4 * 1024,
            HandleTableSize = 1 * 1024,
        };
    };
};

using FilePathHash = uint32_t;
using FileTypeHash = uint32_t;
using FileSize = uint32_t;
using FileID = uint16_t; // value 0 means invalid id
using ContainerID = uint8_t;
using StringID = uint32_t;

using Char = char;
using String = std::string; // std::basic_string<Char>;
using CString = Char *;
using ConstCString = const Char *;

using DynamicFIDTable = std::vector<FileID>;

} // namespace StarVFS
