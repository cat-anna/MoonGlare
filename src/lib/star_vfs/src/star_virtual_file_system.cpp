#include "svfs/star_virtual_file_system.hpp"
#include "file_table.hpp"
#include "file_table_proxy.hpp"
#include "svfs/vfs_exporter.hpp"
#include "svfs/vfs_module.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <fmt/format.h>
#include <orbit_logger.h>
#include <ranges>
#include <set>
#include <svfs/vfs_container.hpp>
#include <svfs/vfs_module_interface.hpp>
#include <unordered_map>
#include <vector>

namespace MoonGlare::StarVfs {

namespace {
FileInfoTable::value_type FileEntryToFileTableEntry(const FileEntry *child) {
    FileInfoTable::value_type entry;
    entry.file_name = child->file_name;
    entry.file_path_hash = child->file_path_hash;
    entry.parent_path_hash = child->parent_path_hash;
    entry.is_directory = child->IsDirectory();
    entry.is_hidden = child->IsHidden();
    entry.file_resource_id = child->resource_id;
    return entry;
}
} // namespace

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
    Impl(iClassRegister *class_register, iStarVfsHooks *hooks = nullptr)
        : class_register(class_register), hooks(hooks) {
        mounted_containers.push_back({}); // allocate id=0, this is invalid id
        loaded_modules.push_back({});     // allocate id=0, this is invalid id
    }

    iVfsContainer *CreateContainer(const std::string_view &container_class,
                                   const VariantArgumentMap &arguments) override {
        mounted_containers.push_back({});
        auto &container_info = mounted_containers.back();
        try {
            container_info.file_table_interface = std::make_unique<FileTableProxy>(
                static_cast<uint32_t>(mounted_containers.size() - 1), &file_table);

            container_info.instance = class_register->CreateContainerObject(
                std::string(container_class), container_info.file_table_interface.get(), arguments);
            if (!container_info.instance)
                throw std::runtime_error("Failed to create container object");

            container_info.instance->ReloadContainer();

            AddLog(Debug, fmt::format("Mounted container of class {}", container_class));
            if (hooks) {
                hooks->OnContainerMounted(container_info.instance.get());
            }
        } catch (const std::exception &e) {
            AddLog(Error, fmt::format("Failed to mount container of class {}: {}", container_class,
                                      e.what()));
            container_info.Reset();
        }
        return container_info.instance.get();
    };

    iVfsModule *LoadModule(const std::string_view &module_class,
                           const VariantArgumentMap &arguments) {
        loaded_modules.push_back({});
        auto &module_info = loaded_modules.back();
        try {
            module_info.instance =
                class_register->CreateModuleObject(std::string(module_class), this, arguments);
            if (!module_info.instance)
                throw std::runtime_error("Failed to create module object");
            module_info.instance->Execute();
            AddLog(Debug, fmt::format("Loaded module of class {}", module_class));
        } catch (const std::exception &e) {
            AddLog(Error,
                   fmt::format("Failed to load module of class {}: {}", module_class, e.what()));
            module_info.Reset();
        }
        return module_info.instance.get();
    }

    std::unique_ptr<iVfsExporter> CreateExporter(const std::string_view &module_class,
                                                 const VariantArgumentMap &arguments) {
        try {
            auto instance =
                class_register->CreateExporterObject(std::string(module_class), this, arguments);
            if (!instance)
                throw std::runtime_error("Failed to create exporter object");
            AddLog(Debug, fmt::format("Created exporter of class {}", module_class));
            return instance;
        } catch (const std::exception &e) {
            AddLog(Error,
                   fmt::format("Failed to load module of class {}: {}", module_class, e.what()));
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

    const FileEntry *GetFileByPath(const std::string &file_path) const override {
        //
        return file_table.FindFileByPath(file_path);
    }

    bool ReadFile(const FileEntry *file, std::string &file_data) const override {
        if (file == nullptr) {
            return false;
        }

        auto container = GetContainer(file->container_id);
        if (container == nullptr) {
            AddLogf(Error, "Failed to get container of %u %u", file->file_path_hash,
                    file->container_id);
            return false;
        }

        return container->ReadFileContent(file->container_file_id, file_data);
    };

    bool WriteFile(const FileEntry *file, const std::string &file_data) override {
        if (file == nullptr) {
            return false;
        }

        auto container = GetContainer(file->container_id);
        if (container == nullptr) {
            AddLogf(Error, "Failed to get container of %u %u", file->file_path_hash,
                    file->container_id);
            return false;
        }

        return container->WriteFileContent(file->container_file_id, file_data);
    };

    bool IsDirectory(const FileEntry *file_entry) const override {
        if (file_entry == nullptr) {
            return false;
        }
        return file_entry->IsDirectory();
    }

    std::string GetFullPath(const FileEntry *file_entry) const override {
        if (file_entry == nullptr) {
            return "";
        }
        return file_entry->GetFullPath();
    }

    FileResourceId GetResourceId(const FileEntry *file_entry) const override {
        if (file_entry == nullptr) {
            return 0;
        }
        return file_entry->resource_id;
    }

    bool IsHidden(const FileEntry *file_entry) const override {
        if (file_entry == nullptr) {
            return 0;
        }
        return file_entry->IsHidden();
    }

    void SetHidden(const FileEntry *file_entry, bool value) override {
        if (file_entry == nullptr) {
            return;
        }
        file_entry->is_hidden = value;
    }

    bool EnumerateFile(const FileEntry *file_entry, EnumerateFileFunctor &functor,
                       bool recursive) const override {
        if (file_entry == nullptr) {
            return false;
        }
        for (auto &child : file_entry->children) {
            if (!functor(child.get(), std::string_view(child->file_name))) {
                return false;
            }
            if (recursive && child->IsDirectory()) {
                if (!EnumerateFile(child.get(), functor, recursive)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool EnumeratePath(const std::string_view &start_path, EnumerateFileFunctor &functor,
                       bool recursive) const override {
        auto *parent_file = file_table.FindFileByPath(start_path);
        if (!parent_file) {
            return false;
        }

        return EnumerateFile(parent_file, functor, recursive);
    };

    bool EnumerateFileTable(EnumerateFileFunctor &functor) const {
        for (const auto &item : file_table.GetResourceMap()) {
            if (!functor(item.second, item.second->file_name)) {
                return false;
            }
        }
        return true;
    };

    std::string DumpStructure() const {
        std::stringstream output;

        auto dump_linear = [&](auto vector) {
            std::sort(vector.begin(), vector.end(),
                      [](const auto &a, const auto &b) { return a.first < b.first; });
            for (const auto &item : vector) {
                output << item.second->String() << std::endl;
            }
        };

        output << "============== file tree ==============\n";
        std::function<void(int, const FileEntry *)> generator;
        output << "file_path_hash   | resource_id      | name\n";
        generator = [&](int level, const FileEntry *file) {
            if (file == nullptr) {
                return;
            }

            if (file->file_name.empty()) {
                return;
            }

            std::string level_str;
            for (int i = 0; i < level; ++i) {
                level_str += "|  ";
            }

            output << fmt::format("{:016x} | {:016x} | {}> {}", file->file_path_hash,
                                  file->resource_id, level_str, file->file_name)
                   << std::endl;

            if (file->IsDirectory()) {
                for (auto &item : file->children) {
                    generator(level + 1, item.get());
                }
            }
        };

        for (auto &item : file_table.GetRootFile()->children) {
            generator(0, item.get());
        }

        output << "\n";
        output << "============== files by resource id ==============\n";
        output << FileEntry::StringHeader() << "\n";
        dump_linear(file_table.GetResourceIdMap());

        output << "\n";
        output << "============== files by path id ==============\n";
        output << FileEntry::StringHeader() << "\n";
        dump_linear(file_table.GetFilePathHashMap());

        output << "\n";
        return output.str();
    }

    std::vector<MountedContainerEntry> mounted_containers;
    std::vector<ModuleEntry> loaded_modules;
    FileTable file_table;

    iClassRegister *const class_register;
    iStarVfsHooks *const hooks;
};

StarVirtualFileSystem::StarVirtualFileSystem(iClassRegister *class_register, iStarVfsHooks *hooks)
    : impl(std::make_unique<Impl>(class_register, hooks)) {
}

StarVirtualFileSystem::~StarVirtualFileSystem() {
}

iVfsContainer *StarVirtualFileSystem::MountContainer(std::string_view container_class,
                                                     const VariantArgumentMap &arguments) {

    return impl->CreateContainer(container_class, arguments);
};

iVfsModule *StarVirtualFileSystem::LoadModule(std::string_view module_class,
                                              const VariantArgumentMap &arguments) {
    return impl->LoadModule(module_class, arguments);
}

std::unique_ptr<iVfsExporter>
StarVirtualFileSystem::CreateExporter(std::string_view module_class,
                                      const VariantArgumentMap &arguments) {
    return impl->CreateExporter(module_class, arguments);
}

bool StarVirtualFileSystem::ReadFileByPath(std::string_view path, std::string &file_data) const {
    auto *file = impl->file_table.FindFileByPath(path);
    if (file == nullptr) {
        AddLogf(Error, "Failed to find file %s : %llu", path.data(), Hasher::Hash(path));
        return false;
    }

    return impl->ReadFile(file, file_data);
};

bool StarVirtualFileSystem::ReadFileByResourceId(FileResourceId resource,
                                                 std::string &file_data) const {
    auto *file = impl->file_table.FindFileByResourceId(resource);
    if (file == nullptr) {
        AddLog(Error, fmt::format("Failed to find resource {:x}", resource));
        return false;
    }

    return impl->ReadFile(file, file_data);
}

FileResourceId StarVirtualFileSystem::GetResourceByPath(std::string_view path) const {
    auto *file = impl->file_table.FindFileByPath(path);
    if (file == nullptr) {
        AddLogf(Error, "Failed to find file %s : %llu", path.data(), Hasher::Hash(path));
        return kInvalidResourceId;
    }
    return file->resource_id;
}

std::string StarVirtualFileSystem::GetNameOfResource(FileResourceId resource,
                                                     bool wants_full_path) const {
    auto *file = impl->file_table.FindFileByResourceId(resource);
    if (file == nullptr) {
        AddLogf(Error, "Failed to find resource %llu", resource);
        return "";
    }
    if (wants_full_path) {
        return file->GetFullPath();
    } else {
        return file->file_name;
    }
}

bool StarVirtualFileSystem::WriteFileByPath(std::string_view path, const std::string &file_data) {
    auto *file = impl->file_table.FindFileByPath(path);
    if (file == nullptr) {
        AddLogf(Error, "Failed to find file %s : %llu", path.data(), Hasher::Hash(path));
        return false;
    }

    return impl->WriteFile(file, file_data);
}

bool StarVirtualFileSystem::EnumeratePath(std::string_view path,
                                          FileInfoTable &result_file_table) const {
    result_file_table.clear();
    iVfsModuleInterface::EnumerateFileFunctor functor = [&result_file_table](const FileEntry *child,
                                                                             auto) -> bool {
        result_file_table.emplace_back(FileEntryToFileTableEntry(child));
        return true;
    };
    return impl->EnumeratePath(path, functor, false);
};

bool StarVirtualFileSystem::FindFilesByExt(std::string_view ext,
                                           FileInfoTable &result_file_table) const {
    result_file_table.clear();
    iVfsModuleInterface::EnumerateFileFunctor functor = [&](const FileEntry *child, auto) -> bool {
        if (boost::ends_with(child->file_name, ext)) {
            result_file_table.emplace_back(FileEntryToFileTableEntry(child));
        }
        return true;
    };
    return impl->EnumerateFileTable(functor);
}

std::string StarVirtualFileSystem::DumpStructure() const {
    return impl->DumpStructure();
}

} // namespace MoonGlare::StarVfs