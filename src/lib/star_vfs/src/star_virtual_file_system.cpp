#include "svfs/star_virtual_file_system.hpp"
#include "file_table.hpp"
#include "file_table_proxy.hpp"
#include "svfs/vfs_exporter.hpp"
#include "svfs/vfs_module.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
#include <svfs/vfs_container.hpp>
#include <svfs/vfs_module_interface.hpp>
#include <unordered_map>
#include <vector>


namespace MoonGlare::StarVfs {

struct MountedContainerEntry {
    std::unique_ptr<iVfsContainer> instance;
    std::unique_ptr<iFileTableInterface> file_table_interface;

    void Reset() {
        file_table_interface.reset();
        instance.reset();
    }
};

struct ModuleEntry {
    std::unique_ptr<iVfsModule> instance;

    void Reset() { instance.reset(); }
};

struct StarVirtualFileSystem::Impl : public iVfsModuleInterface {
    Impl(iClassRegister *class_register) : class_register(class_register) {
        mounted_containers.push_back({}); // allocate id=0, this is invalid id
        loaded_modules.push_back({});     // allocate id=0, this is invalid id
    }

    iVfsContainer *CreateContainer(const std::string_view &container_class,
                                   const VariantArgumentMap &arguments) override {
        mounted_containers.push_back({});
        auto &container_info = mounted_containers.back();
        try {
            container_info.file_table_interface =
                std::make_unique<FileTableProxy>(static_cast<uint32_t>(mounted_containers.size() - 1), &file_table);

            container_info.instance = class_register->CreateContainerObject(
                std::string(container_class), container_info.file_table_interface.get(), arguments);

            container_info.instance->ReloadContainer();

            AddLog(Debug, fmt::format("Mounted container of class {}", container_class));
        } catch (const std::exception &e) {
            AddLog(Error, fmt::format("Failed to mount container of class {}: {}", container_class, e.what()));
            container_info.Reset();
        }
        return container_info.instance.get();
    };

    iVfsModule *LoadModule(const std::string_view &module_class, const VariantArgumentMap &arguments) {
        loaded_modules.push_back({});
        auto &module_info = loaded_modules.back();
        try {
            module_info.instance = class_register->CreateModuleObject(std::string(module_class), this, arguments);
            AddLog(Debug, fmt::format("Loaded module of class {}", module_class));
        } catch (const std::exception &e) {
            AddLog(Error, fmt::format("Failed to load module of class {}: {}", module_class, e.what()));
            module_info.Reset();
        }
        return module_info.instance.get();
    }

    std::unique_ptr<iVfsExporter> CreateExporter(const std::string_view &module_class,
                                                 const VariantArgumentMap &arguments) {
        try {
            auto instance = class_register->CreateExporterObject(std::string(module_class), this, arguments);
            AddLog(Debug, fmt::format("Created exporter of class {}", module_class));
            return instance;
        } catch (const std::exception &e) {
            AddLog(Error, fmt::format("Failed to load module of class {}: {}", module_class, e.what()));
        }
        return nullptr;
    }

    iVfsContainer *GetContainer(uint32_t container) const override {
        if (container >= mounted_containers.size()) {
            return nullptr;
        }
        return mounted_containers[container].instance.get();
    }

    const FileTable *GetFileTable() const { return &file_table; };
    iClassRegister *GetClassRegister() const override { return class_register; };

    bool ReadFile(const FileEntry *file, std::string &file_data) const override {
        if (file == nullptr) {
            return false;
        }

        auto container = GetContainer(file->container_id);
        if (container == nullptr) {
            AddLogf(Error, "Failed to get container of %u %u", file->file_path_hash, file->container_id);
            return false;
        }

        return container->ReadFileContent(file->container_file_id, file_data);
    };

    std::vector<MountedContainerEntry> mounted_containers;
    std::vector<ModuleEntry> loaded_modules;
    FileTable file_table;

    iClassRegister *const class_register;
};

StarVirtualFileSystem::StarVirtualFileSystem(iClassRegister *class_register)
    : impl(std::make_unique<Impl>(class_register)) {}

StarVirtualFileSystem::~StarVirtualFileSystem() {}

iVfsContainer *StarVirtualFileSystem::MountContainer(const std::string_view &container_class,
                                                     const VariantArgumentMap &arguments) {

    return impl->CreateContainer(container_class, arguments);
};

iVfsModule *StarVirtualFileSystem::LoadModule(const std::string_view &module_class,
                                              const VariantArgumentMap &arguments) {
    return impl->LoadModule(module_class, arguments);
}

std::unique_ptr<iVfsExporter> StarVirtualFileSystem::CreateExporter(const std::string_view &module_class,
                                                                    const VariantArgumentMap &arguments) {
    return impl->CreateExporter(module_class, arguments);
}

bool StarVirtualFileSystem::ReadFileByPath(const std::string &path, std::string &file_data) const {
    auto *file = impl->file_table.FindFileByPath(path);
    if (file == nullptr) {
        AddLogf(Error, "Failed to find file %s : %llu", path.c_str(), Hasher::Hash(path));
        return false;
    }

    return impl->ReadFile(file, file_data);
};

bool StarVirtualFileSystem::EnumeratePath(const std::string_view &path, FileInfoTable &result_file_table) {
    result_file_table.clear();
    auto *parent_file = impl->file_table.FindFileByPath(path);

    if (!parent_file) {
        return false;
    }

    for (auto &child : parent_file->children) {
        FileInfoTable::value_type entry{child->file_name, child->IsDirectory()};
        result_file_table.emplace_back(std::move(entry));
    }

    return true;
};

} // namespace MoonGlare::StarVfs