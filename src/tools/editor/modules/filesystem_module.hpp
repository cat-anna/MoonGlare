
#pragma once

#include <QFileSystemWatcher>
#include <QTimer>
#include <runtime_modules.h>
#include <unordered_set>
#include <writable_file_system.h>
// #include <qobject.h>
// #include "Notifications.h"
// #include <Foundation/iFileSystem.h>
// #include <StarVFS/core/nStarVFS.h>
// #include <ToolBase/Module.h>
// #include <iFileProcessor.h>

namespace MoonGlare::Tools::Editor::Modules {

// class AsyncFileProcessor;

class FileSystemModule : public QObject,
                         public iModule,
                         public iWritableFileSystem
// , public iFileSystem
{
    Q_OBJECT;

public:
    FileSystemModule(SharedModuleManager modmgr);
    virtual ~FileSystemModule();

    bool Initialize() override;
    bool PostInit() override;
    bool Finalize() override;

    void OpenPath(std::string new_root_path);
    const std::string &GetOpenedPath() { return root_path; }
    bool IsPathOpened() const { return svfs_instance != nullptr; };

    bool WriteFileByPath(const std::string &path, const std::string &file_data) override;

    // iReadOnlyFileSystem
    bool ReadFileByPath(const std::string &path, std::string &file_data) const override;
    bool ReadFileByResourceId(FileResourceId resource, std::string &file_data) const override;
    bool EnumeratePath(const std::string_view &path, FileInfoTable &result_file_table) const override;
    FileResourceId GetResourceByPath(const std::string_view &path) const override;
    std::string GetNameOfResource(FileResourceId resource, bool wants_full_path = true) const override;

    // StarVFS::SharedStarVFS GetVFS() { return m_VFS; }

    // bool GetFileData(const std::string &uri, StarVFS::ByteTable &data);
    // bool SetFileData(const std::string &uri, StarVFS::ByteTable &data);

    // bool CreateFile(const std::string &uri);
    // bool CreateDirectory(const std::string &uri);
    // bool FileExists(const std::string &uri) const override;

    // std::string TranslateToSystem(const std::string &uri);

    // bool OpenFile(StarVFS::ByteTable &FileData, const std::string& uri) override {
    //     return GetFileData(uri, FileData);
    // }
    // bool OpenFile(StarVFS::ByteTable &FileData, StarVFS::FileID fid) override {
    //     __debugbreak();
    //     throw false;
    // };

    // void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out)  override {
    //     __debugbreak();
    //     throw false;
    // };
    // std::string GetFileName(StarVFS::FileID fid) const override {
    //     __debugbreak();
    //     throw false;
    // };
    // std::string GetFullFileName(StarVFS::FileID fid) const override {
    //     __debugbreak();
    //     throw false;
    // };

    // bool EnumerateFolder(const std::string& Path, FileInfoTable &FileTable, bool Recursive) override;

    // void QueueFileProcessing(const std::string &URI);
public slots:
    void Reload();
signals:
    void Changed();

private:
    std::string root_path;

    // 	std::mutex m_Mutex;
    // 	StarVFS::SharedStarVFS m_VFS;
    // 	Module::SharedDataModule m_Module;
    // 	std::unordered_map<std::string, std::vector<std::weak_ptr<QtShared::iFileProcessorInfo>>> m_ExtFileProcessorList;
    //     QtShared::SharedJobProcessor jobProcessor;
    //     QtShared::SharedJobFence jobFence;

    QFileSystemWatcher filesystem_watcher;
    QTimer watcher_timeout;
    std::unordered_set<std::string> changed_paths;

    // 	bool TranslateURI(const std::string &uri, std::string &out);

    void UpdateFsWatcher();
    void HandleDirectoryChanged(const std::string &path);
    void RefreshChangedPaths();

    struct StarVfsInstance;
    std::unique_ptr<StarVfsInstance> svfs_instance;
};

} // namespace MoonGlare::Tools::Editor::Modules
