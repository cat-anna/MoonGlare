#pragma once

#include <svfs/hashes.hpp>

namespace MoonGlare::StarVfs {

class iFileTableInterface;

enum class AccessMode {
    ReadOnly,
    ReadWrite,
};

class iVfsContainer {
public:
    explicit iVfsContainer(iFileTableInterface *fti);
    virtual ~iVfsContainer() = default;

    virtual void ReloadContainer() = 0;

    virtual bool ReadFileContent(FilePathHash container_file_id, std::string &file_data) const = 0;
    virtual bool WriteFileContent(FilePathHash container_file_id, const std::string &file_data) { return false; };

    /** Returns container_file_id that can be used in ReadFileContent/WriteFileContent */
    virtual FilePathHash FindFile(const std::string &relative_path) const { return false; }

    // /** Get string which may be used to reopen container (filename, base path or ip address) */
    // virtual String GetContainerURI() const = 0;

    virtual AccessMode GetAccessMode() const { return AccessMode::ReadOnly; };

    // ContainerID GetContainerID() const { return GetFileTableInterface()->GetContainerID(); }

    // virtual bool CreateFile(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr);
    // virtual bool CreateDirectory(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr);

    ///Get filename by cfid. Returns nullptr if cfid is not valid or function is not supported
    // virtual ConstCString GetFileName(FileID ContainerFID) const;

    /** Enumerate files in container. cfids are in container domain. Returns false if not supported */
    // virtual bool EnumerateFiles(ContainerFileEnumFunc filterFunc) const;
protected:
    iFileTableInterface *const file_table_interface;
};

} // namespace MoonGlare::StarVfs
