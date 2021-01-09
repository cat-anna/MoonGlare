#include "svfs/content_info_exporter.hpp"
#include "file_entry.hpp"
#include "file_table.hpp"
#include "svfs/path_utils.hpp"
#include "svfs/vfs_module_interface.hpp"
#include <fmt/format.h>
#include <fstream>
#include <orbit_logger.h>
#include <stdexcept>

namespace MoonGlare::StarVfs {

//-------------------------------------------------------------------------------------------------

ContentInfoExporter::ContentInfoExporter(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments)
    : iVfsExporter(module_interface) {

    arguments.get_to(output_path, "output_path", std::string());
    arguments.get_to(root_point, "root_point", std::string());
    arguments.get_to(include_hidden_files, "include_hidden_files", false);
}

ContentInfoExporter::~ContentInfoExporter() {
    //
}

void ContentInfoExporter::StartExport() {
    if (output_path.empty()) {
        throw std::runtime_error("Invalid content info output file");
    }

    std::ofstream out(output_path);

    std::function<void(const FileEntry *, const std::string &)> generator;
    generator = [&](const FileEntry *file, const std::string &parent_path) {
        if (file == nullptr) {
            return;
        }

        if (file->file_name.empty() || (file->IsHidden() && !include_hidden_files)) {
            return;
        }

        std::string my_path = parent_path;
        my_path += "/";
        my_path += file->file_name;

        out << my_path << "\n";
        out << "\tfile://" << my_path << "\n";
        out << fmt::format("\thash://{:016x} ({})\n", file->file_path_hash, file->file_path_hash);
        out << fmt::format("\tparent {}\n", file->parent_path_hash);
        if (file->resource_id > 0) {
            out << fmt::format("\tres://{:016x} ({})\n", file->resource_id, file->resource_id);
        }

        if (file->IsDirectory()) {
            for (auto &item : file->children) {
                generator(item.get(), my_path);
            }
        }
    };

    auto *root_file = module_interface->GetFileTable()->GetRootFile();
    for (auto &item : root_file->children) {
        generator(item.get(), "");
    }
};

} // namespace MoonGlare::StarVfs
