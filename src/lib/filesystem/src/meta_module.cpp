#include "svfs/meta_module.h"
#include "file_entry.hpp"
#include "file_table.hpp"
#include "svfs/dynamic_file_container.hpp"
#include "svfs/vfs_module_interface.hpp"
#include <fmt/format.h>
#include <functional>
#include <orbit_logger.h>

namespace MoonGlare::StarVfs {

namespace {

std::string DumpHashMap(std::vector<std::pair<FilePathHash, FileEntry *>> data) {
    std::stringstream out;

    std::sort(data.begin(), data.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

    static const char *kLineFormat = "{:16} {:16s} {:16s} {:1s} {}\n";
    out << fmt::format(kLineFormat, "PARENT_HASH", "FILE_HASH", "CONTENT_HASH", "F", "FILENAME");

    for (const auto &item : data) {
        out << fmt::format(kLineFormat,                                            //
                           fmt::format("{:016x}", item.second->parent_path_hash),  //
                           fmt::format("{:016x}", item.second->file_path_hash),    //
                           fmt::format("{:016x}", item.second->file_content_hash), //
                           item.second->IsDirectory() ? "D" : " ",                 //
                           item.second->file_name                                  //
        );
    };

    out << "entries count: " << data.size() << "\n";
    return out.str();
}

} // namespace

MetaModule::MetaModule(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments)
    : iVfsModule(module_interface) {

    VariantArgumentMap sub_container_args;
    auto raw_container = module_interface->CreateContainer(DynamicFileContainer::kClassName, sub_container_args);
    dynamic_container = static_cast<DynamicFileContainer *>(raw_container);

    if (dynamic_container == nullptr) {
        AddLog(Error, "Failed to create DynamicFileContainer");
        throw std::runtime_error("Failed to create DynamicFileContainer");
    }

    dynamic_container->AddFile("/.file_path_hash_map", [this] { return DumpFilePathHashMap(); });
    dynamic_container->AddFile("/.file_content_hash_map", [this] { return DumpFileContentHashMap(); });
    dynamic_container->AddFile("/.file_tree", [this] { return DumpFileTree(); });
    dynamic_container->ReloadContainer();

    AddLog(Info, "StarVfs meta-module created");
}

MetaModule::~MetaModule(){};

std::string MetaModule::DumpFileTree() const {
    std::stringstream out;

    static const char *kLineFormat = "{:16} {:16s} {:16s} {:1s} {}{}\n";
    out << fmt::format(kLineFormat, "PARENT_HASH", "FILE_HASH", "CONTENT_HASH", "F", "TREE", "");

    size_t file_count = 0;
    std::function<void(const FileEntry *, int)> printer;
    printer = [&printer, &out, &file_count](const FileEntry *file, int current_level) {
        if (file == nullptr) {
            return;
        }
        if (current_level >= 0) {
            std::string level;
            for (int i = 0; i < current_level; ++i) {
                level += "| ";
            }
            ++file_count;
            out << fmt::format(kLineFormat,                                     //
                               fmt::format("{:016x}", file->parent_path_hash),  //
                               fmt::format("{:016x}", file->file_path_hash),    //
                               fmt::format("{:016x}", file->file_content_hash), //
                               file->IsDirectory() ? "D" : " ",                 //
                               level,                                           //
                               file->file_name                                  //
            );
        }
        for (auto &item : file->children) {
            printer(item.get(), current_level + 1);
        }
    };

    printer(module_interface->GetFileTable()->GetRootFile(), -1);
    out << "File count: " << file_count << "\n";
    return out.str();
}

std::string MetaModule::DumpFilePathHashMap() const {
    return DumpHashMap(module_interface->GetFileTable()->GetFilePathHashMap());
}

std::string MetaModule::DumpFileContentHashMap() const {
    return DumpHashMap(module_interface->GetFileTable()->GetFileContentHashMap());
}

#if 0

struct StatisticsMetaFile : public Containers::BaseDynamicFileInterface {
    StatisticsMetaFile(StarVFS *svfs) : m_svfs(svfs) { StarVFSAssert(svfs); }
    void GenerateContent(std::ostream &o) override {
        auto ft = m_svfs->GetFileTable();

        o << "Allocated files: " << ft->GetAllocatedFileCount() << "\n";
        o << "Loaded containers: " << (int)m_svfs->GetContainerCount() << "\n";
        o << "Loaded modules: " << (int)m_svfs->GetModuleCount() << "\n";
        o << "\n";
        //	o << "Registered container types:\n";
        //	o << "Registered module types:\n";
        //	o << "Registered exporter types:\n";
    }
};

#endif

} // namespace MoonGlare::StarVfs
