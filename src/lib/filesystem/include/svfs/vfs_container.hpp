#pragma once

#include <svfs/definitions.hpp>
#include <svfs/hashes.hpp>

namespace MoonGlare::StarVfs {

class iFileTableInterface;

class iVfsContainer {
public:
    explicit iVfsContainer(iFileTableInterface *fti);
    virtual ~iVfsContainer() = default;

    virtual void ReloadContainer() = 0;

    virtual bool ReadFileContent(FilePathHash container_file_id, std::string &file_data) const = 0;

    // /** Get string which may be used to reopen container (filename, base path or ip address) */
    // virtual String GetContainerURI() const = 0;
    // virtual RWMode GetRWMode() const = 0;

    // ContainerID GetContainerID() const { return GetFileTableInterface()->GetContainerID(); }

    // /** Find file in container. ContainerFileName shall be relative to container. Function is may be slow. */
    // virtual FileID FindFile(const String& ContainerFileName) const = 0;

    // /** Container shall not do any buffering. */
    // virtual bool GetFileData(FileID ContainerFID, ByteTable &out) const = 0;
    // virtual bool SetFileData(FileID ContainerFID, const ByteTable &in) const;

    // virtual bool CreateFile(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr);
    // virtual bool CreateDirectory(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr);

    // ///Get filename by cfid. Returns nullptr if cfid is not valid or function is not supported
    // virtual ConstCString GetFileName(FileID ContainerFID) const;

    // /** Enumerate files in container. cfids are in container domain. Returns false if not supported */
    // virtual bool EnumerateFiles(ContainerFileEnumFunc filterFunc) const;

    // static bool CanOpen(const String&) { return false; }
    // static CreateContainerResult CreateFor(StarVFS *svfs, const String& MountPoint, const String& Location) { return
    // CreateContainerResult(VFSErrorCode::InternalError, nullptr); }
protected:
    iFileTableInterface *const file_table_interface;
};

} // namespace MoonGlare::StarVfs
