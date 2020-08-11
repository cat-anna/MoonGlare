
#include <fmt/format.h>
#include <orbit_logger.h>
#include <svfs/dynamic_file_container.hpp>
#include <tuple>

namespace MoonGlare::StarVfs {

namespace {

std::tuple<std::string_view, std::string_view> GetParentAndFileName(const std::string_view &path) {
    auto last_slash = path.find_last_of("/");
    if (last_slash == std::string_view::npos) {
        return {{}, path};
    }
    std::string_view parent = path.substr(0, last_slash);
    std::string_view file_name = path.substr(last_slash + 1);
    return {parent, file_name};
}

} // namespace

bool DynamicFileContainer::FunctorFileInterface::ReadFileContent(std::string &out) const {
    try {
        out = functor();
        return true;
    } catch (const std::exception &e) {
        AddLogf(Error, "File content generation failed: %s", e.what());
    }
    return false;
}

//-------------------------------------------------------------------------------------------------

DynamicFileContainer::DynamicFileContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments)
    : iVfsContainer(fti){};

void DynamicFileContainer::ReloadContainer() {
    using ContainerFileEntry = iFileTableInterface::ContainerFileEntry;
    std::vector<ContainerFileEntry> request_table;
    request_table.reserve(file_map.size());

    for (auto &item : file_map) {
        ContainerFileEntry entry;
        entry.file_name = item.second.file_name;

        entry.container_file_id = item.first;
        entry.parent_path_hash = item.second.file_parent_hash;
        entry.file_path_hash = item.second.file_path_hash;
        entry.file_content_hash = item.second.file_content_hash;

        request_table.emplace_back(std::move(entry));
    }

    if (!file_table_interface->RegisterFileStructure(request_table)) {
        AddLog(Error, "Reloading dynamic file container failed");
    }
};

bool DynamicFileContainer::ReadFileContent(FilePathHash container_file_id, std::string &file_data) const {
    auto it = file_map.find(container_file_id);
    if (it == file_map.end()) {
        AddLog(Error, "File does not exists");
        return false;
    }

    return it->second.file_interface->ReadFileContent(file_data);
};

void DynamicFileContainer::AddFile(const std::string_view &path, SharedDynamicFileInterface shared_file_interface) {
    if (shared_file_interface == nullptr) {
        AddLog(Error, "Attempt to add file with null interface");
        return;
    }

    auto [parent_path, file_name] = GetParentAndFileName(path);

    FileInfo fi;
    fi.file_name = std::string(file_name);
    fi.file_interface = std::move(shared_file_interface);
    fi.file_path_hash = Hasher::Hash(path);
    fi.file_parent_hash = Hasher::Hash(parent_path);

    file_map.insert_or_assign(fi.file_path_hash, std::move(fi));
}

void DynamicFileContainer::AddFile(const std::string_view &path, FunctorFileInterface::Functor functor) {
    AddFile(path, std::make_shared<FunctorFileInterface>(std::move(functor)));
}

void DynamicFileContainer::AddDirectory(const std::string_view &path) { file_table_interface->CreateDirectory(path); }

} // namespace MoonGlare::StarVfs
