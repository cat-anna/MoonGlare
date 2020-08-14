
#pragma once

#include <string>
#include <string_view>
#include <svfs/hashes.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

class iFileTableInterface {
public:
    iFileTableInterface() = default;
    virtual ~iFileTableInterface() = default;

    struct ContainerFileEntry {
        std::string file_name;
        // uint32_t file_size;
        // bool is_directory;
        ContainerFileId container_file_id{0};
        FilePathHash parent_path_hash{0};
        FilePathHash file_path_hash{0};
        FileResourceId resource_id{0};

        bool is_directory{false};
    };

    virtual bool RegisterFileStructure(const std::vector<ContainerFileEntry> &Structure) = 0;

    virtual void CreateDirectory(const std::string &path) = 0;
    void CreateDirectory(const std::string_view &path) {
        CreateDirectory(std::string(path)); // TODO
    }
};

} // namespace MoonGlare::StarVfs
