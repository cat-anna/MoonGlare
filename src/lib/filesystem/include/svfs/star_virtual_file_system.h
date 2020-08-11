#pragma once

#include "file_table_interface.h"
#include "readonly_file_system.h"
#include "star_vfs.h"
#include "svfs/definitions.h"
#include "variant_argument_map.hpp"
#include <any>
#include <string>
#include <string_view>
#include <svfs/class_register.hpp>
#include <template_class_list.h>

namespace MoonGlare::StarVfs {

class iVfsModule;

class StarVirtualFileSystem : public iStarVfs {
public:
    explicit StarVirtualFileSystem(iClassRegister *class_register);
    ~StarVirtualFileSystem() override;

    iVfsContainer *MountContainer(const std::string_view &container_class,
                                  const VariantArgumentMap &arguments) override;
    iVfsModule *LoadModule(const std::string_view &module_class, const VariantArgumentMap &arguments) override;

    bool ReadFileByPath(const std::string &path, std::string &file_data) override;

    bool EnumeratePath(const std::string_view &path, FileInfoTable &result_file_table) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    // template <class T, class... ARGS> CreateContainerResult CreateContainer(const String &MountPoint, ARGS...
    // args) {
    //     static_assert(std::is_base_of<Containers::iContainer, T>::value, "Invalid container class!");
    //     Container c = std::make_unique<T>(NewFileTableInterface(MountPoint), std::forward<ARGS>(args)...);
    //     auto cptr = c.get();
    //     auto r = MountContainer(std::move(c), MountPoint);
    //     if (r != VFSErrorCode::Success)
    //         cptr = nullptr;
    //     return CreateContainerResult(r, cptr);
    // }
    // Containers::iContainer *GetContainer(ContainerID cid);
    // ContainerID GetContainerCount() const;
    // bool CloseContainer(ContainerID cid);

    // VFSErrorCode OpenContainer(const String &ContainerFile, const String &MountPoint = "/");

    // /** Debug function. Prints all files in human-readable format. */
    // void DumpStructure(std::ostream &out) const;
    // void DumpFileTable(std::ostream &out) const;
    // void DumpHashTable(std::ostream &out) const;

    // bool ForcePath(const String &path);

    // FileID FindFile(const String &FileName);
    // /// LastName is ptr inside to FileName arg
    // FileID FindOneBeforeLastFile(const String &FileName, ConstCString *LastName = nullptr);
    // bool IsFileValid(FileID fid) const;
    // bool IsFileDirectory(FileID fid) const;
    // String GetFullFilePath(FileID fid) const;
    // /** Get file relative path to ParentFID. fid and ParentFID must have line relation. */
    // String GetFilePath(FileID fid, FileID ParentFID) const;
    // CString GetFileName(FileID fid) const;
    // FileSize GetFileSize(FileID fid) const;
    // bool DeleteFile(FileID fid);
    // bool GetFileData(FileID fid, ByteTable &data);
    // std::pair<ContainerID, FileID> GetFileContainerInfo(FileID fid) const;

    // FileHandle OpenFile(const String &FileName, RWMode ReadMode = RWMode::R,
    //                     OpenMode FileMode = OpenMode::OpenExisting);
    // FileHandle OpenFile(FileID fid, RWMode ReadMode = RWMode::R);
    // FileHandle CreateDirectory(const String &FileName, RWMode ReadMode = RWMode::R);

    // void FindFilesByTypeHash(const CString ext, DynamicFIDTable &table);
    // void FindFilesByTypeHash(FileTypeHash fth, DynamicFIDTable &table);

    // template <class T, class... ARGS> Modules::iModule *AddModule(ARGS... args) {
    //     static_assert(std::is_base_of<Modules::iModule, T>::value, "Invalid module class!");
    //     return InsertModule(std::make_unique<T>(this, std::forward<ARGS>(args)...));
    // }
    // size_t GetModuleCount() const;
    // Modules::iModule *GetModule(size_t mid);

    // template <class T, class... ARGS> std::unique_ptr<T> CreateExporter(ARGS... args) {
    //     static_assert(std::is_base_of<Exporters::iExporter, T>::value, "Invalid exporter class!");
    //     return std::make_unique<T>(this, std::forward<ARGS>(args)...);
    // }

    // FileTable *GetFileTable() { return m_FileTable.get(); }
    // HandleTable *GetHandleTable();

    // /** Set callback for StrVFS facade. Returns previous one. */
    // StarVFSCallback *SetCallback(StarVFSCallback *newone);

    // Register *GetRegister();

protected:
    //	virtual bool CanLoadContainer(iContainer *container);
    //	virtual void OnContaierLoaded(iContainer *container);
private:
    // unsigned m_Flags;
    // struct Internals;
    // std::unique_ptr<Internals> m_Internals;
    // StarVFSCallback *m_Callback;

    // VFSErrorCode MountContainer(Container c, String MountPoint);
    // VFSErrorCode ReloadContainer(ContainerID cid, bool FirstMount);
    // Containers::FileTableInterface *NewFileTableInterface(const String &MountPoint, bool Force = false);
    // Modules::iModule *InsertModule(std::unique_ptr<Modules::iModule> module);
};

} // namespace MoonGlare::StarVfs