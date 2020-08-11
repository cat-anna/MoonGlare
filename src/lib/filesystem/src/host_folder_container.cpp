#include "svfs/host_folder_container.h"
#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <stdexcept>

namespace MoonGlare::StarVfs {

namespace fs = std::filesystem;
using ContainerFileEntry = iFileTableInterface::ContainerFileEntry;

namespace {

static const std::filesystem::path kHfcMeta{".hash"};

bool ReadHostFileContent(const fs::path &path, std::string &file_data) {
    try {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path, std::ios_base::binary);
        std::size_t sz = static_cast<std::size_t>(std::filesystem::file_size(path));
        file_data.resize(sz);
        file.read(file_data.data(), sz);
        return true;
    } catch (...) {
        return false;
    }
}

struct FileContentInfo {
    FileContentHash content_hash;
};

void to_json(nlohmann::json &j, const FileContentInfo &p) { j = {{"content_hash", p.content_hash}}; }
void from_json(const nlohmann::json &j, FileContentInfo &p) { j.at("content_hash").get_to(p.content_hash); }

template <typename T> T ReadJsonFromFile(const fs::path &path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path, std::ios_base::binary);
    nlohmann::json json;
    file >> json;
    return json.get<T>();
}

template <typename T> void WriteJsonFromFile(const fs::path &path, const T &value) {
    nlohmann::json json = value;
    std::ofstream file;
    file.open(path, std::ios_base::binary | std::ios::out);
    file << nlohmann::json(value);
}

bool ScanPath(fs::path base_path, const std::string &mount_point, bool hash_content,
              std::vector<ContainerFileEntry> &request_table, HostFolderContainer::FileMapper &new_file_mapper,
              HostFolderContainer::FileMapper &previous_file_mapper) {

    for (auto current_file : fs::recursive_directory_iterator(base_path)) {
        auto current_path = current_file.path();
        if (current_path.extension() == kHfcMeta) {
            continue;
        }

        auto relative_path = fs::relative(current_file.path(), base_path);
        auto parent_path = relative_path.parent_path();

        std::string relative_string = mount_point + relative_path.generic_string();
        std::string parent_string = mount_point + parent_path.generic_string();
        if (parent_string == "/") {
            parent_string = "";
        }

        auto local_hash = Hasher::Hash(current_file.path().generic_string());
        auto relative_hash = Hasher::Hash(relative_string);
        auto parent_hash = Hasher::Hash(parent_string);

        // AddLog(Error, fmt::format("{} {}", relative_hash, relative_string));

        auto collision = new_file_mapper.find(local_hash);
        if (collision != new_file_mapper.end()) {
            AddLog(Error, fmt::format("Collision detected for HostFolderContainer:{} at path {}",
                                      base_path.generic_string(), relative_string));
            continue;
        }

        new_file_mapper[local_hash] = {current_file.path()};
        previous_file_mapper.erase(local_hash);

        ContainerFileEntry entry;
        entry.is_directory = current_file.is_directory();

        if (!entry.is_directory) {
            auto meta_file = current_path;
            meta_file += kHfcMeta;
            try {
                if (std::filesystem::is_regular_file(meta_file)) {
                    auto meta_content = ReadJsonFromFile<FileContentInfo>(meta_file);
                    entry.file_content_hash = meta_content.content_hash;
                }
            } catch (const std::exception &e) {
                AddLog(Error, fmt::format("Meta read failed for {} : {}", meta_file.generic_string(), e.what()));
            }
            try {
                if (hash_content && entry.file_content_hash == 0) {
                    FileContentInfo meta_content;
                    std::string file_content;
                    AddLog(Info, fmt::format("Meta write hasing: {}", current_path.generic_string()));
                    if (!ReadHostFileContent(current_path, file_content)) {
                        throw std::runtime_error("Source file read failed");
                    }
                    meta_content.content_hash = Hasher::Hash(file_content);
                    WriteJsonFromFile(meta_file, meta_content);
                }
            } catch (const std::exception &e) {
                AddLog(Error, fmt::format("Meta write failed for {} : {}", meta_file.generic_string(), e.what()));
            }
        }

        entry.file_name = current_file.path().filename().generic_string();
        entry.container_file_id = local_hash;
        entry.file_path_hash = relative_hash;
        entry.parent_path_hash = parent_hash;

        entry.file_content_hash = 0;

        request_table.push_back(std::move(entry));
    }

    request_table.shrink_to_fit();
    return true;
}

} // namespace

//-------------------------------------------------------------------------------------------------

HostFolderContainer::HostFolderContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments)
    : iVfsContainer(fti) {
    mountpoint = arguments.get("mountpoint", "");
    while (!mountpoint.empty() && mountpoint.back() == '/') {
        mountpoint.pop_back();
    }
    mountpoint += "/";

    host_path = std::filesystem::absolute(arguments.get("host_path"));
    hash_content = arguments.get("hash_content", "1") == "1";
}

//-------------------------------------------------------------------------------------------------

// String FolderContainer::GetContainerURI() const { return m_Path; }
// RWMode FolderContainer::GetRWMode() const { return RWMode::RW; }

//-------------------------------------------------------------------------------------------------

void HostFolderContainer::ReloadContainer() {
    AddLog(FSEvent,
           fmt::format("Reloading host folder container '{}' mounted at {}", host_path.generic_string(), mountpoint));

    std::vector<ContainerFileEntry> request_table;
    FileMapper old_file_mapper = file_mapper;
    FileMapper new_file_mapper;

    ScanPath(host_path, mountpoint, hash_content, request_table, new_file_mapper, old_file_mapper);

    if (!old_file_mapper.empty()) {
        // TODO: erase removed files
        AddLog(Warning, "Not implemented");
    }

    file_mapper.swap(new_file_mapper);
    file_table_interface->CreateDirectory(mountpoint);
    if (!file_table_interface->RegisterFileStructure(request_table)) {
        AddLog(Error, "Reloading host folder container failed");
    }
}

bool HostFolderContainer::ReadFileContent(FilePathHash container_file_id, std::string &file_data) const {
    auto it = file_mapper.find(container_file_id);
    if (it == file_mapper.end()) {
        AddLog(Error, "File does not exists");
        return false;
    }

    const FileEntry &entry = it->second;

    return ReadHostFileContent(entry.host_path, file_data);
}

#if 0

//-------------------------------------------------------------------------------------------------

bool FolderContainer::SetFileData(FileID ContainerFID, const ByteTable &in) const {
	if (ContainerFID >= m_FileEntry.size() || ContainerFID == 0)
		return false;

	auto &f = m_FileEntry[ContainerFID];
	std::ofstream outf(f.m_FullPath.c_str(), std::ios::out | std::ios::binary);
	outf.write(in.c_str(), in.byte_size());
	outf.close();

	return true;
}

FileID FolderContainer::FindFile(const String& ContainerFileName) const {
	for (auto it = m_FileEntry.begin(), jt = m_FileEntry.end(); it != jt; ++it)
		if (it->m_SubPath == ContainerFileName)
			return static_cast<FileID>(it - m_FileEntry.begin());

	return 0;
}

bool FolderContainer::EnumerateFiles(ContainerFileEnumFunc filterFunc) const {
	for (auto &entry: m_FileEntry) {
		auto cfid = static_cast<FileID>(&entry - &m_FileEntry[0]);
		FileFlags flags;
		flags.intval = 0;
		flags.Directory = entry.m_Type == FileType::Directory;
		flags.Valid = true;
//		using ContainerFileEnumFunc = std::function<bool(ConstCString fname, FileFlags flags, FileID CFid, FileID ParentCFid)>;
		
		if (!filterFunc((ConstCString)entry.m_SubPath.c_str(), flags, cfid, (FileID)0/*parentcfid*/))
			break;
	}
	return true;
}

bool FolderContainer::CreateFile(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput) {
	Entry *f;
	size_t index;
	if (!AllocateFile(ContainerBaseFID, Name, f, index))
		return false;

	f->m_Type = FileType::File;
	auto fti = GetFileTableInterface();
	StarVFSAssert(fti);
	if (!fti->CreateFile(f->m_GlobalFid, static_cast<FileID>(index), 0)) {
		STARVFSErrorLog("Failed to create file for %s", f->m_SubPath.c_str());
		//todo: cfid is not deallocated;
		return false;
	}
	if (GlobalFIDOutput)
		*GlobalFIDOutput = f->m_GlobalFid;
	std::ofstream of(f->m_FullPath, std::ios::out);
	of.close();
	return true;
}

bool FolderContainer::CreateDirectory(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput) {
	Entry *f;
	size_t index;
	if (!AllocateFile(ContainerBaseFID, Name, f, index))
		return false;

	f->m_Type = FileType::Directory;
	auto fti = GetFileTableInterface();
	StarVFSAssert(fti);
	if (!fti->CreateDirectory(f->m_GlobalFid, static_cast<FileID>(index))) {
		STARVFSErrorLog("Failed to create file for %s", f->m_SubPath.c_str());
		//todo: cfid is not deallocated;
		return false;
	}
	if (GlobalFIDOutput)
		*GlobalFIDOutput = f->m_GlobalFid;

	try {
		boost::filesystem::create_directory(f->m_FullPath);
	}
	catch (...) { 		
		return false;//todo: handle this
	}

	return true;
}

bool FolderContainer::AllocateFile(FileID ContainerBaseFID, ConstCString Name, Entry *&out, size_t &index) {
	if (GetRWMode() < RWMode::W || ContainerBaseFID >= m_FileEntry.size() || ContainerBaseFID == 0)
		return false;

	auto &basef = m_FileEntry[ContainerBaseFID];
	std::string path = basef.m_FullPath + "/" + Name;

	auto fti = GetFileTableInterface();
	StarVFSAssert(fti);
	Entry f;
	f.m_FullPath = path;
	f.m_FileSize = 0;
	f.m_SubPath = basef.m_SubPath + "/" + Name;

	f.m_GlobalFid = fti->AllocFileID((CString)f.m_SubPath.c_str());
	if (!f.m_GlobalFid) {
		STARVFSErrorLog("Failed to alloc fileid for %s", f.m_SubPath.c_str());
		return false;
	}

    index = m_FileEntry.size();
	m_FileEntry.emplace_back(std::move(f));
	out = &m_FileEntry.back();

	return true;
}

bool FolderContainer::GetFileSystemPath(FileID cfid, std::string &out) {
    if (cfid >= m_FileEntry.size() || cfid == 0)
        return false;
    auto &f = m_FileEntry[cfid];
    out = f.m_FullPath;
    return true;
}

//-------------------------------------------------------------------------------------------------

template <class T>
bool FolderContainer::EnumerateFolder(const String &Path, T func) {
	using boost::filesystem::directory_iterator;
	using boost::filesystem::recursive_directory_iterator;
	boost::filesystem::path p(Path);
	if (!boost::filesystem::is_directory(Path))
		return false;
	
	try {
		recursive_directory_iterator it(Path);

		for (; it != recursive_directory_iterator();) {
			auto item = it->path();
			FileType type;

			String fullPath = item.string();

			if (boost::filesystem::is_regular_file(item))
				type = FileType::File;
			else
				type = FileType::Directory;

			func(fullPath, type);

			while(it != recursive_directory_iterator())
				try {
					++it;
					break;
				} 
				catch (...) {
					it.no_push();
				}
			}
	}
	catch (const std::exception &e) {
		STARVFSErrorLog("Exception: %s", e.what());
		return true;
	}

	return true;
}

#endif

} // namespace MoonGlare::StarVfs
