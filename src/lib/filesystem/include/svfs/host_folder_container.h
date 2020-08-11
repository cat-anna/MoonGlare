#pragma once

#include "file_table_interface.h"
#include "svfs/vfs_container.h"
#include "variant_argument_map.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace MoonGlare::StarVfs {

class HostFolderContainer : public iVfsContainer {
public:
    static constexpr const char *kClassName = "host_folder";

    HostFolderContainer(iFileTableInterface *fti, const VariantArgumentMap &arguments);
    ~HostFolderContainer() override = default;

    void ReloadContainer() override;

    bool ReadFileContent(FilePathHash container_file_id, std::string &file_data) const override;

    // virtual bool RegisterContent() const override;

    // virtual FileID GetFileCount() const override;

    // virtual String GetContainerURI() const override;

    // virtual RWMode GetRWMode() const override;

    // virtual bool GetFileData(FileID ContainerFID, ByteTable &out) const override;
    // virtual bool SetFileData(FileID ContainerFID, const ByteTable &in) const override;
    // virtual FileID FindFile(const String &ContainerFileName) const override;
    // virtual bool EnumerateFiles(ContainerFileEnumFunc filterFunc) const override;
    // virtual bool CreateFile(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr) override;
    // virtual bool CreateDirectory(FileID ContainerBaseFID, ConstCString Name,
    //                              FileID *GlobalFIDOutput = nullptr) override;

    // bool GetFileSystemPath(FileID cfid, std::string &out);

    // static bool CanOpen(const String &Location);
    // static CreateContainerResult CreateFor(StarVFS *svfs, const String &MountPoint, const String &Location);

    struct FileEntry {
        std::filesystem::path host_path;
    };

    using FileMapper = std::unordered_map<FilePathHash, FileEntry>;

private:
    std::filesystem::path host_path;
    std::string mountpoint;
    bool hash_content{false};

    FileMapper file_mapper;

    // enum class FileType {
    //     File,
    //     Directory,
    // };

    // struct Entry {
    //     FileType m_Type;
    //     FileID m_GlobalFid;
    //     String m_FullPath;
    //     String m_SubPath;
    //     uint64_t m_FileSize;
    // };

    // template <class T> static bool EnumerateFolder(const String &Path, T func);

    // String m_Path;

    // std::vector<Entry> m_FileEntry;

    // bool ScanPath();
    // bool AllocateFile(FileID ContainerBaseFID, ConstCString Name, Entry *&out, size_t &index);
};

} // namespace MoonGlare::StarVfs
