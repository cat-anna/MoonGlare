
#pragma once

#include <string>
#include <string_view>
#include <svfs/hashes.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

class iFileTableInterface {
public:
    iFileTableInterface() = default;
    virtual ~iFileTableInterface() = default;

    struct ContainerFileEntry {
        std::string file_name;
        // uint32_t file_size;
        // bool is_directory;
        ContainerFileId container_file_id{0};
        FilePathHash parent_path_hash{0};
        FilePathHash file_path_hash{0};
        FileContentHash file_content_hash{0};

        bool is_directory{false};
    };

    virtual bool RegisterFileStructure(const std::vector<ContainerFileEntry> &Structure) = 0;

    virtual void CreateDirectory(const std::string &path) = 0;
    void CreateDirectory(const std::string_view &path) {
        CreateDirectory(std::string(path)); // TODO
    }

    // bool EnsureReserve(FileID count);

    // FileID FindFile(const CString InternalFullPath);
    // FileID FindFile(FilePathHash PathHash);

    // FileID AllocFileID(const CString InternalFullPath);
    // /** Forces full path to be valid. Several directories may be created if necessary. Function is slow, should not
    // be used */ FileID ForceAllocFileID(const CString InternalFullPath); FileID AllocFileID(FileID Parent,
    // FilePathHash PathHash, const CString FileName);

    // FileID GetRootID() const { return m_MountPoint; }
    // bool IsMoutedToRoot() const { return m_MountPoint == 1; }

    // bool CreateFile(FileID fid, FileID cfid, FileSize Size) const;
    // bool CreateDirectory(FileID fid, FileID cfid) const;
    // //bool CreateLink()
    // bool DeleteFile(FileID fid);

    // /** function fails if container is not owner of fid pr fid is not a regular file */
    // bool UpdateFileSize(FileID fid, FileSize NewSize);

    // bool IsFileValid(FileID fid) const;
    // bool IsDirectory(FileID fid) const;
    // bool IsFile(FileID fid) const;

    // CString GetFileName(FileID fid) const { return m_Owner->GetFileName(fid); }
    // String GetFileFullPath(FileID fid) const { return m_Owner->GetFileFullPath(fid); }

    // ContainerID GetContainerID() const { return m_CID; }
    // FileTable* GetFileTable() { return m_Owner; }
private:
    // FileTable *m_Owner;
    // FileID m_MountPoint;
    // ContainerID m_CID;
};

} // namespace MoonGlare::StarVfs
