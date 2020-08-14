#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <svfs/file_table_interface.hpp>
#include <svfs/hashes.hpp>
#include <svfs/variant_argument_map.hpp>
#include <svfs/vfs_container.hpp>
#include <unordered_map>

namespace MoonGlare::StarVfs {

class DynamicFileContainer : public iVfsContainer {
public:
    static constexpr const char *kClassName = "dynamic_file";

    class DynamicFileInterface {
    public:
        explicit DynamicFileInterface() = default;
        virtual ~DynamicFileInterface() = default;

        virtual bool ReadFileContent(std::string &out) const = 0;
    };

    class FunctorFileInterface : public DynamicFileInterface {
    public:
        using Functor = std::function<std::string()>;
        explicit FunctorFileInterface(Functor functor) : functor(functor) {}
        virtual ~FunctorFileInterface() = default;

        bool ReadFileContent(std::string &out) const override;

    private:
        Functor functor;
    };

    using SharedDynamicFileInterface = std::shared_ptr<DynamicFileInterface>;

    DynamicFileContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments);
    ~DynamicFileContainer() override = default;

    void ReloadContainer() override;

    bool ReadFileContent(FilePathHash container_file_id, std::string &file_data) const override;

    void AddFile(const std::string_view &path, SharedDynamicFileInterface shared_file_interface);
    void AddFile(const std::string_view &path, FunctorFileInterface::Functor functor);
    void AddDirectory(const std::string_view &path);

private:
    struct FileInfo {
        SharedDynamicFileInterface file_interface;

        std::string file_name;
        FilePathHash file_path_hash{0};
        FilePathHash file_parent_hash{0};
        FileResourceId resource_id{0};
    };
    std::unordered_map<FilePathHash, FileInfo> file_map;
};

} // namespace MoonGlare::StarVfs
