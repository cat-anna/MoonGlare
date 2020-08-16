
#include "filesystem_module.hpp"
#include "svfs/star_virtual_file_system.hpp"
#include <QDirIterator>
#include <svfs/host_folder_container.hpp>
#include <svfs/svfs_class_register.hpp>
#include <svfs_modules/assimp_container.hpp>
#include <svfs_modules/assimp_importer_module.hpp>

namespace MoonGlare::Tools::Editor::Modules {

struct FileSystemModule::StarVfsInstance {
    StarVfsInstance(const std::string &root_path) : file_system(&class_register) {
        class_register.RegisterAll();
        class_register.RegisterModuleClass<SvfsModules::AssimpImporterModule>();
        class_register.RegisterContainerClass<SvfsModules::AssimpContainer>();

        VariantArgumentMap args;
        args.set("host_path", root_path);
        args.set("mount_point", std::string(""));
        args.set("generate_resource_id", true);
        args.set("store_resource_id", true);
        file_system.MountContainer("host_folder", args);
    }
    StarVfs::SvfsClassRegister class_register;
    StarVfs::StarVirtualFileSystem file_system;
};

//-----------------------------------------------------------------------------

ModuleClassRegister::Register<FileSystemModule> FileSystemReg("FileSystemModule");

FileSystemModule::FileSystemModule(SharedModuleManager modmgr)
    : iModule(std::move(modmgr)), filesystem_watcher(this), watcher_timeout(this) {

    // m_VFS = std::make_shared<StarVFS::StarVFS>();
    // jobFence = std::make_shared<QtShared::MutexJobFence>();

    // connect(Notifications::Get(), SIGNAL(ProjectChanged(Module::SharedDataModule)), this,
    //         SLOT(ProjectChanged(Module::SharedDataModule)));

    connect(&filesystem_watcher, &QFileSystemWatcher::directoryChanged,
            [this](const QString &path) { HandleDirectoryChanged(path.toStdString()); });
    connect(&filesystem_watcher, &QFileSystemWatcher::fileChanged,
            [this](const QString &path) { HandleDirectoryChanged(path.toStdString()); });

    watcher_timeout.setInterval(1000);
    watcher_timeout.setSingleShot(true);
    connect(&watcher_timeout, &QTimer::timeout, [this]() { RefreshChangedPaths(); });
}

FileSystemModule::~FileSystemModule() {
    // jobFence.reset();
    // m_VFS.reset();
}

bool FileSystemModule::Initialize() {
    if (!iModule::Initialize())
        return false;

    GetModuleManager()->RegisterInterface<iWritableFileSystem>(this);
    GetModuleManager()->RegisterInterface<iReadOnlyFileSystem>(this);

    return true;
}

bool FileSystemModule::PostInit() {
    // for (auto mod : GetModuleManager()->QueryInterfaces<QtShared::iFileProcessorInfo>()) {
    //     for (auto &it : mod.m_Interface->GetSupportedTypes()) {
    //         m_ExtFileProcessorList[it].push_back(mod.m_Interface);
    //         AddLogf(Info, "Registered file processor %s -> '%s'", it.c_str(),
    //                 typeid(*mod.m_Module).name());
    //     }
    // }
    // jobProcessor = GetModuleManager()->QueryModule<QtShared::iJobProcessor>();
    return true;
}

bool FileSystemModule::Finalize() {
    // m_ExtFileProcessorList.clear();
    // jobProcessor.reset();
    return true;
}

void FileSystemModule::OpenPath(std::string new_root_path) {
    if (svfs_instance) {
        throw std::runtime_error("Not possible "); //TODO: better message
    }

    root_path = std::move(new_root_path);
    svfs_instance = std::make_unique<StarVfsInstance>(root_path);

    // std::unique_lock<QtShared::iJobFence> fenceLock(*jobFence);
    UpdateFsWatcher();
    Reload();
}

//-----------------------------------------------------------------------------

// bool FileSystemModule::GetFileData(const std::string &uri, StarVFS::ByteTable &data) {
//     LOCK_MUTEX(m_Mutex);
//     std::string path;
//     if (!TranslateURI(uri, path)) {
//         return false;
//     }
//     auto fid = m_VFS->FindFile(path);
//     return m_VFS->GetFileData(fid, data);
// }

// bool FileSystemModule::SetFileData(const std::string &uri, StarVFS::ByteTable &data) {
//     std::string path;
//     if (!TranslateURI(uri, path)) {
//         return false;
//     }

//     LOCK_MUTEX(m_Mutex);
//     auto fid = m_VFS->FindFile(path);
//     if (!m_VFS->IsFileValid(fid)) {
//         // TODO: todo
//         return false;
//     }

//     auto h = m_VFS->OpenFile(fid, StarVFS::RWMode::W);
//     if (!h) {
//         // TODO: todo
//         return false;
//     }

//     auto ret = h.SetFileData(data);
//     h.Close();

//     QueueFileProcessing(uri);

//     return ret;
// }

// bool FileSystemModule::CreateFile(const std::string &uri) {
//     if (uri.empty())
//         return false;

//     std::string path;
//     if (!TranslateURI(uri, path)) {
//         return false;
//     }

//     LOCK_MUTEX(m_Mutex);
//     auto h = m_VFS->OpenFile(path, StarVFS::RWMode::RW, StarVFS::OpenMode::CreateNew);

//     if (!h) {
//         AddLog(Error, "Failed to create file " << uri);
//         return false;
//     }

//     h.Close();

//     emit Changed();
//     return true;
// }

// bool FileSystemModule::CreateDirectory(const std::string &uri) {
//     if (uri.empty())
//         return false;

//     std::string path;
//     if (!TranslateURI(uri, path)) {
//         return false;
//     }

//     LOCK_MUTEX(m_Mutex);
//     auto h = m_VFS->CreateDirectory(path);

//     if (!h) {
//         AddLog(Error, "Failed to create directory " << uri);
//         return false;
//     }
//     h.Close();

//     emit Changed();
//     return true;
// }

// bool FileSystemModule::TranslateURI(const std::string &uri, std::string &out) {
//     if (uri.empty()) {
//         AddLog(Error, "Empty URI!");
//         return false;
//     }
//     auto pos = uri.find("://");
//     if (pos == std::string::npos) {
//         __debugbreak();
//         return false;
//     }
//     auto start = pos + 3;

//     switch (Space::Utils::MakeHash32(uri.c_str(), pos)) {
//     case "file"_Hash32:
//         out = uri.substr(start);
//         return true;
//     default:
//         AddLog(Error, "Invalid URI: " << uri);
//         return false;
//     }
// }

// std::string FileSystemModule::TranslateToSystem(const std::string &uri) {
//     std::string r;
//     TranslateURI(uri, r);
//     return m_BasePath + r;
// }

//-----------------------------------------------------------------------------

// void FileSystemModule::QueueFileProcessing(const std::string &URI) {
//     std::string fpath;
//     if (!TranslateURI(URI, fpath)) {
//         AddLogf(Error, "Invalid file URI: %s", URI.c_str());
//         return;
//     }

//     auto pos = fpath.rfind(".");
//     if (pos == std::string::npos) {
//         AddLogf(Error, "Cannot find file extension: %s", fpath.c_str());
//         return;
//     }

//     auto ext = fpath.substr(pos + 1);
//     auto pit = m_ExtFileProcessorList.find(ext);
//     if (pit == m_ExtFileProcessorList.end() || pit->second.empty()) {
//         //	AddLogf(Info, "There is no registered processors which can handle file %s(%s)", fpath.c_str(),
//         //URI.c_str());
//         return;
//     }

//     for (auto &pinfo : pit->second) {
//         auto proc = pinfo.lock();
//         if (proc) {
//             auto processor = proc->CreateFileProcessor(URI);
//             if (!processor) {
//                 AddLogf(Error, "Failed to create file processor: %s", typeid(*proc.get()).name());
//                 continue;
//             }
//             AddLogf(Info, "Queued processing of file %s", URI.c_str());
//             processor->SetFence(jobFence);
//             jobProcessor->Queue(processor);
//             // m_QueuedFileProcessors.push_back(std::move(processor));
//         }
//     }
// }

void FileSystemModule::HandleDirectoryChanged(const std::string &path) {
    AddLogf(Info, "Directory changed: %s", path.c_str());
    changed_paths.insert(path);
    watcher_timeout.start();
}

void FileSystemModule::RefreshChangedPaths() {
    AddLogf(Info, "Processing changed paths");
    // std::unique_lock<QtShared::iJobFence> fenceLock(*jobFence);

    // for (auto &item : changedPaths) {
    //     if (item.find(m_BasePath) != 0) {
    //         continue;
    //     }
    //     if (boost::FileSystemModule::is_regular_file(item)) {
    //         auto uri = "file://" + item.substr(m_BasePath.size() - 1);
    //         QueueFileProcessing(uri);
    //         continue;
    //     }
    //     if (boost::FileSystemModule::is_directory(item)) {
    //         // TODO
    //     }
    // }
    // changedPaths.clear();
    // Reload();
}

//-----------------------------------------------------------------------------

void FileSystemModule::UpdateFsWatcher() {
    filesystem_watcher.removePaths(filesystem_watcher.directories());
    QStringList list;
    QDirIterator it(QString::fromStdString(root_path), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto v = it.next();
        std::string fn = QDir(v).dirName().toStdString();
        if (fn != "." && fn != "..") {
            list.push_back(v);
        }
    }
    list.push_back(QString::fromStdString(root_path));
    filesystem_watcher.addPaths(list);
}

void FileSystemModule::Reload() {
    if (!svfs_instance) {
        return;
    }

    // if (jobProcessor->GetQueuedJobCount() > 0) {
    //     return;
    // }

    // LOCK_MUTEX(m_Mutex);
    // std::unique_lock<QtShared::iJobFence> fenceLock(*jobFence);

    bool processFiles = false;
    // if (m_VFS->GetContainerCount() == 0) {
    // } else {
    //     for (size_t i = 0; i < m_VFS->GetContainerCount(); ++i) {
    //         auto c = m_VFS->GetContainer(static_cast<::StarVFS::ContainerID>(i + 1));
    //         if (!c) {
    //             AddLogf(Error, "Failed to get base container");
    //         } else {
    //             c->ReloadContainer();
    //             c->RegisterContent();
    //         }
    //     }
    // }

    emit Changed();

    // StarVFS::HandleEnumerateFunc processitem;
    // auto svfs = GetVFS();
    // processitem = [this, &svfs, &processitem](StarVFS::FileID fid) -> bool {
    //     auto h = svfs->OpenFile(fid);
    //     if (!h)
    //         return true;

    //     if (h.IsDirectory()) {
    //         h.EnumerateChildren(processitem);
    //         return true;
    //     }

    //     auto fileuri = StarVFS::MakeFileURI(h.GetFullPath().c_str());
    //     QueueFileProcessing(fileuri);

    //     h.Close();

    //     return true;
    // };

    // if (processFiles) {
    //     auto root = svfs->OpenFile("/");
    //     root.EnumerateChildren(processitem);
    //     root.Close();
    // }
}

//-----------------------------------------------------------------------------

bool FileSystemModule::ReadFileByPath(const std::string &path, std::string &file_data) const {
    if (!svfs_instance) {
        return false;
    }
    return svfs_instance->file_system.ReadFileByPath(path, file_data);
}

bool FileSystemModule::EnumeratePath(const std::string_view &path,
                                     FileInfoTable &result_file_table) const {
    if (!svfs_instance) {
        return false;
    }
    return svfs_instance->file_system.EnumeratePath(path, result_file_table);
}

//-----------------------------------------------------------------------------

// bool FileSystemModule::EnumerateFolder(const std::string &Path, FileInfoTable &FileTable,
//                                        bool Recursive) {
//     auto ParentFID = m_VFS->FindFile(Path);

//     if (!m_VFS->IsFileValid(ParentFID)) {
//         AddLogf(Error, "Failed to open '%s' for enumeration", Path.c_str());
//         return false;
//     }

//     FileTable.reserve(1024); // because thats why
//     StarVFS::HandleEnumerateFunc svfsfunc;

//     svfsfunc = [this, &svfsfunc, &FileTable, Recursive, ParentFID](StarVFS::FileID fid) -> bool {
//         if (ParentFID != fid) {
//             auto path = m_VFS->GetFilePath(fid, ParentFID);

//             FileInfo fi;
//             fi.m_IsFolder = m_VFS->IsFileDirectory(fid);
//             fi.m_RelativeFileName = m_VFS->GetFilePath(fid, ParentFID);
//             fi.m_FileName = m_VFS->GetFileName(fid);
//             fi.m_FID = fid;
//             FileTable.push_back(fi);
//         }

//         if ((Recursive || ParentFID == fid) && m_VFS->IsFileDirectory(fid)) {
//             auto handle = m_VFS->OpenFile(fid);
//             if (!handle.EnumerateChildren(svfsfunc)) {
//                 handle.Close();
//                 return true;
//             }
//             handle.Close();
//         }

//         return true;
//     };

//     return svfsfunc(ParentFID);
// }

// bool FileSystemModule::FileExists(const std::string &uri) const {
//     std::string u = uri;
//     auto pos = u.find("://");
//     if (pos != std::string::npos)
//         u = u.substr(pos + 3);
//     auto fid = m_VFS->FindFile(u);
//     return m_VFS->IsFileValid(fid);
// }

} // namespace MoonGlare::Tools::Editor::Modules
