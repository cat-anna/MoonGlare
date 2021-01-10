#include "svfs/zip_exporter.hpp"
#include "file_entry.hpp"
#include "file_table.hpp"
#include "svfs/path_utils.hpp"
#include "svfs/vfs_module_interface.hpp"
#include "zip_container_manifest.hpp"
#include <build_configuration.hpp>
#include <fmt/format.h>
#include <fstream>
#include <json_helpers.hpp>
#include <libzippp.h>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <stdexcept>
#include <vector>

namespace MoonGlare::StarVfs {

//-------------------------------------------------------------------------------------------------

ZipExporter::ZipExporter(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments)
    : iVfsExporter(module_interface) {

    arguments.get_to(output_path, "output_path", std::string());
    arguments.get_to(root_point, "root_point", std::string());
    arguments.get_to(include_hidden_files, "include_hidden_files", false);
    arguments.get_to(generate_resource_id, "generate_resource_id", true);
    arguments.get_to(deflate, "deflate", false);
}

ZipExporter::~ZipExporter() {
    //
}

void ZipExporter::StartExport() {
    using namespace libzippp;

    if (output_path.empty()) {
        throw std::runtime_error("Invalid zip output file");
    }

    ZipArchive zf(output_path);
    ZipContainerManifest manifest{};
    zf.open(ZipArchive::NEW);
    std::list<std::string> loaded_files;

    AddLog(Info, fmt::format("Starting export to zip {}", output_path));
    AddLog(Verbose, fmt::format("Deflate: {}", deflate));
    AddLog(Verbose, fmt::format("Include hidden files: {}", include_hidden_files));
    AddLog(Verbose, fmt::format("Generate resource id: {}", generate_resource_id));

    size_t file_index = 0;
    size_t total_size = 0;
    size_t hashed_files = 0;

    std::function<void(const FileEntry *, const std::string &)> generator;
    generator = [&](const FileEntry *file, const std::string &parent_path) {
        if (file == nullptr) {
            return;
        }

        if (file->file_name.empty() || (file->IsHidden() && !include_hidden_files)) {
            return;
        }

        std::string my_path = parent_path;
        if (!my_path.empty())
            my_path += "/";
        my_path += file->file_name;

        if (file->IsDirectory()) {
            for (auto &item : file->children) {
                generator(item.get(), my_path);
            }
            return;
        }

        ++file_index;
        if ((file_index % 50) == 0) {
            AddLog(Verbose, fmt::format("Reading file {}", file_index));
        }

        std::string file_data;
        if (!module_interface->ReadFile(file, file_data)) {
            AddLog(Error, "Failed to read file to export to zip");
            // throw std::runtime_error("Failed to read file to export to zip");
            return;
        }

        auto resource_id = file->resource_id;
        if (resource_id == 0 && generate_resource_id) {
            resource_id = Hasher::Hash(file_data);
            ++hashed_files;
        }

        if (resource_id != 0) {
            manifest.file_entries[my_path] = {resource_id};
        }

        zf.addData(my_path, file_data.c_str(), file_data.size());
        zf.getEntry(my_path).setCompressionEnabled(deflate);
        total_size += file_data.size();
        loaded_files.emplace_back(std::move(file_data));
    };

    auto *root_file = module_interface->GetFileTable()->GetRootFile();

    for (auto &item : root_file->children) {
        generator(item.get(), "");
    }

    auto manifest_data = to_json_string(manifest, kDebugBuild);
    zf.addData(kContainerManifestFileName, manifest_data.c_str(), manifest_data.size());
    zf.getEntry(kContainerManifestFileName).setCompressionEnabled(deflate);

    AddLog(Verbose, fmt::format("Closing archive {}", output_path));
    AddLog(Verbose, fmt::format("File count: {}", file_index));
    AddLog(Verbose, fmt::format("Hashed file count: {}", hashed_files));
    AddLog(Verbose, fmt::format("Total source file size: {:.02f} MiB", total_size / 1024.0 / 1024.0));

    zf.close();

    AddLog(Info, fmt::format("Export to zip {} completed", output_path));
};

} // namespace MoonGlare::StarVfs
