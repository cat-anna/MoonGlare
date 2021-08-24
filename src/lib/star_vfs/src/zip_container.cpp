#include "svfs/zip_container.hpp"
#include "svfs/path_utils.hpp"
#include "zip_container_manifest.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <json_helpers.hpp>
#include <libzippp.h>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <stdexcept>
#include <vector>

namespace MoonGlare::StarVfs {

using ContainerFileEntry = iFileTableInterface::ContainerFileEntry;

//-------------------------------------------------------------------------------------------------

struct ZipContainer::ZipMapper {
    libzippp::ZipArchive zip_archive;
    std::vector<libzippp::ZipEntry> zip_entries;

    ZipMapper(const std::filesystem::path &zip_file, const std::string &password)
        : zip_archive(zip_file.generic_string(), password) {

        zip_archive.open(libzippp::ZipArchive::ReadOnly);
        zip_entries = zip_archive.getEntries();
    }
    ZipContainerManifest ReadManifest() {
        try {
            auto manifest_entry = zip_archive.getEntry(kContainerManifestFileName);
            if (!manifest_entry.isNull()) {
                return from_json_string<ZipContainerManifest>(manifest_entry.readAsText());
            }
        } catch (const std::exception &e) {
            AddLogf(Error, "Failed to parse container manifest: %s", e.what());
        }
        return {};
    }
};

//-------------------------------------------------------------------------------------------------

ZipContainer::ZipContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments) : iVfsContainer(fti) {
    mount_point = OptimizeMountPointPath(arguments.get<std::string>("mount_point", ""));
    zip_password = arguments.get<std::string>("password", "");
    zip_file_path = std::filesystem::absolute(arguments.get<std::string>("zip_file_path"));
    if (!std::filesystem::is_regular_file(zip_file_path)) {
        throw std::runtime_error("Invalid zip file path: " + zip_file_path.generic_string());
    }
}

//-------------------------------------------------------------------------------------------------

void ZipContainer::ReloadContainer() {
    AddLog(FSEvent,
           fmt::format("Reloading zip container '{}' mounted at {}", zip_file_path.generic_string(), mount_point));
    zip_mapper = std::make_unique<ZipMapper>(zip_file_path, zip_password);
    auto manifest = zip_mapper->ReadManifest();

    std::vector<ContainerFileEntry> request_table;
    request_table.reserve(zip_mapper->zip_entries.size());

    for (size_t index = 0; index < zip_mapper->zip_entries.size(); ++index) {
        auto &entry = zip_mapper->zip_entries[index];

        std::string zip_path = entry.getName();
        if (zip_path == kContainerManifestFileName) {
            continue; // TODO: optimize somehow
        }

        if (entry.isDirectory()) {
            while (!zip_path.empty() && zip_path.back() == '/')
                zip_path.pop_back();
        }

        auto [zip_parent_path, file_name] = GetParentAndFileName(zip_path);

        auto parent_string = JoinPath(mount_point, zip_parent_path);

        auto parent_hash = Hasher::Hash(parent_string);
        auto file_hash = parent_string.empty() ? Hasher::HashTogether(parent_string, file_name)
                                               : Hasher::HashTogether(parent_string, "/", file_name);

        ContainerFileEntry request_entry = {};
        request_entry.file_name = file_name;
        request_entry.container_file_id = static_cast<ContainerFileId>(index);

        auto manifest_entry_it = manifest.file_entries.find(zip_path);
        if (manifest_entry_it != manifest.file_entries.end()) {
            request_entry.resource_id = manifest_entry_it->second.resource_id;
        }

        request_entry.parent_path_hash = parent_hash;
        request_entry.file_path_hash = file_hash;
        request_entry.is_directory = entry.isDirectory();

        request_table.emplace_back(request_entry);
    }

    if (!file_table_interface->RegisterFileStructure(request_table)) {
        AddLog(Error, "Reloading zip container failed");
    }
}

bool ZipContainer::ReadFileContent(FilePathHash container_file_id, std::string &file_data) const {
    if (zip_mapper == nullptr) {
        return false;
    }
    if (container_file_id >= zip_mapper->zip_entries.size()) {
        return false;
    }

    auto &entry = zip_mapper->zip_entries[container_file_id];
    file_data = entry.readAsText();
    return true;
}

FilePathHash ZipContainer::FindFile(const std::string &relative_path) const {
    for (size_t i = 0; i < zip_mapper->zip_entries.size(); ++i) {
        if (zip_mapper->zip_entries[i].getName() == relative_path) {
            return static_cast<FilePathHash>(i);
        }
    }
    return 0;
}

std::string ZipContainer::GetContainerName() const {
    return fmt::format("svfs://{}@{}", kClassName, zip_file_path.generic_string());
}
} // namespace MoonGlare::StarVfs
