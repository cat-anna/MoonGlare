#include "assimp_importer_module.hpp"
#include "svfs/host_file_svfs_manifest.hpp"
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <json_helpers.hpp>
#include <orbit_logger.h>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace MoonGlare::Tools::SvfsModules {

AssimpImporterModule::AssimpImporterModule(StarVfs::iVfsModuleInterface *module_interface,
                                           const VariantArgumentMap &arguments)
    : StarVfs::iVfsModule(module_interface) {
    arguments.get_to(root_point, "root_point", std::string());
    arguments.get_to(module_manager, "module_manager");
    arguments.get_to(use_manifest, "use_manifest", false);
}

AssimpImporterModule::~AssimpImporterModule() {
}

void AssimpImporterModule::Execute() {
    std::vector<std::tuple<const StarVfs::FileEntry *, std::string_view, std::string>> files_found;

    StarVfs::iVfsModuleInterface::EnumerateFileFunctor import_functor = //
        [&files_found](const StarVfs::FileEntry *file, std::string_view file_name) {
            if (file == nullptr) {
                return true;
            }

            auto dot_pos = file_name.find_last_of(".");
            if (dot_pos == std::string_view::npos) {
                return true;
            }

            std::string ext = std::string(file_name.substr(dot_pos));
            boost::to_lower(ext);

            static const std::unordered_set<std::string> kSupportedExtensions = {
                ".3ds",
                ".blend",
                ".fbx",
            };

            if (kSupportedExtensions.count(ext) == 0) {
                return true;
            }

            files_found.emplace_back(file, file_name, std::move(ext));
            return true;
        };

    module_interface->EnumeratePath(std::string_view(root_point), import_functor, true);

    for (auto &entry : files_found) {
        auto &[file, file_name, ext] = entry;
        ProcessFileEntry(file, file_name, ext);
    }
}

void AssimpImporterModule::ProcessFileEntry(const StarVfs::FileEntry *file, std::string_view file_name,
                                            const std::string &ext) {

    auto full_path = module_interface->GetFullPath(file);

    auto already_loaded_it = known_files.find(full_path);
    AssimpContainer *container;
    if (already_loaded_it != known_files.end()) {
        AddLog(Verbose, fmt::format("Found already imported file: {}", full_path));
        container = already_loaded_it->second;
    } else {
        AddLog(Verbose, fmt::format("Found importable by assimp file: {}", full_path));

        auto mount_point = full_path;

        VariantArgumentMap args;
        args.set("mount_point", mount_point);
        args.set("module_manager", std::any(module_manager));

        container =
            dynamic_cast<AssimpContainer *>(module_interface->CreateContainer(AssimpContainer::kClassName, args));
        if (container != nullptr) {
            known_files[full_path] = container;
        }
    }

    if (container == nullptr) {
        AddLogf(Error, "Failed to create container for %s", full_path.c_str());
    }

    std::string file_data;

    if (!module_interface->ReadFile(file, file_data)) {
        AddLogf(Error, "Failed to read file %s", full_path.c_str());
        return;
    }

    auto file_content_hash = StarVfs::Hasher::Hash(file_data);
    StarVfs::HostFileSvfsManifest file_manifest_data;

    auto file_manifest = module_interface->GetFileByPath(full_path + StarVfs::kHostFileSvfsManifestExtension);
    if (file_manifest) {
        std::string manifest_string;
        if (!module_interface->ReadFile(file_manifest, manifest_string)) {
            AddLogf(Error, "Failed to read manifest of file %s", full_path.c_str());
        } else {
            file_manifest_data = from_json_string<StarVfs::HostFileSvfsManifest>(manifest_string);
        }
    }

    try {
        auto resource_id = module_interface->GetResourceId(file);
        if (resource_id == 0) {
            resource_id = file_content_hash;
        }

        if (use_manifest && file_content_hash == file_manifest_data.content_hash &&
            file_manifest_data.content_imported_by == kClassName) {
            AddLogf(Debug, "File %s hash matches - skipping import", full_path.c_str());
            container->LoadFromCache(resource_id, file_data, file_manifest_data, ext);
            return;
        }

        if (use_manifest && !file_manifest_data.content_imported_by.empty() &&
            file_manifest_data.content_imported_by != kClassName) {
            AddLogf(Debug, "File %s already imported by other module - skipping import", full_path.c_str());
            return;
        }

        file_manifest_data.sub_contents = {};
        container->LoadFromMemory(resource_id, file_data, file_manifest_data, ext);

        if (file_manifest != nullptr) {
            file_manifest_data.content_hash = file_content_hash;
            file_manifest_data.hide_original_file = false;
            file_manifest_data.content_imported_by = kClassName;
            module_interface->WriteFile(file_manifest, to_json_string(file_manifest_data, true));
        }
        AddLogf(Debug, "File %s imported", full_path.c_str());
    } catch (const std::exception) {
        AddLogf(Error, "Failed to import %s", full_path.c_str());
    }
}

} // namespace MoonGlare::Tools::SvfsModules
