/*
  * Generated by cppsrc.sh
  * On 2016-07-22 11:09:18,36
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER

#include <QDirIterator>

#include "FileSystem.h"

#include <StarVFS/core/Container/FolderContainer.h>
#include "DataModule.h"
#include "AsyncFileProcessor.h"

namespace MoonGlare {
namespace Editor {

ModuleClassRgister::Register<FileSystem> FileSystemReg("FileSystem");

FileSystem::FileSystem(SharedModuleManager modmgr) :
        iModule(std::move(modmgr)), fsWatcher(this) {

    m_VFS = std::make_shared<StarVFS::StarVFS>();
    jobFence = std::make_shared<QtShared::MutexJobFence>();

    connect(Notifications::Get(), SIGNAL(ProjectChanged(Module::SharedDataModule)), this, SLOT(ProjectChanged(Module::SharedDataModule)));

    connect(&fsWatcher, &QFileSystemWatcher::directoryChanged, [this](const QString &path) { HandleDirectoryChanged(path.toLocal8Bit().begin()); });
    connect(&fsWatcher, &QFileSystemWatcher::fileChanged, [this](const QString &path) { HandleDirectoryChanged(path.toLocal8Bit().begin()); });

    watcherTimeout.setInterval(1000);
    watcherTimeout.setSingleShot(true);
    connect(&watcherTimeout, &QTimer::timeout, [this]() { RefreshChangedPaths(); });

}

FileSystem::~FileSystem() {
}

bool FileSystem::Initialize() {
    if (!iModule::Initialize())
        return false;

    GetModuleManager()->RegisterInterface<iFileSystem>(this);

    return true;
}

bool FileSystem::PostInit() {
    for (auto mod : GetModuleManager()->QuerryInterfaces<QtShared::iFileProcessorInfo>()) {
        for (auto &it : mod.m_Interface->GetSupportedTypes()) {
            m_ExtFileProcessorList[it].push_back(mod.m_Interface);
            AddLogf(Info, "Registered file processor %s -> '%s'", it.c_str(), typeid(*mod.m_Module).name());
        }
    }
    jobProcessor = GetModuleManager()->QuerryModule<QtShared::iJobProcessor>();
    return true;
}

bool FileSystem::GetFileData(const std::string &uri, StarVFS::ByteTable & data) {
    LOCK_MUTEX(m_Mutex);
    std::string path;
    if (!TranslateURI(uri, path)) {
        return false;
    }
    auto fid = m_VFS->FindFile(path);
    return m_VFS->GetFileData(fid, data);
}

bool FileSystem::SetFileData(const std::string & uri, StarVFS::ByteTable & data) {
    std::string path;
    if (!TranslateURI(uri, path)) {
        return false;
    }

    LOCK_MUTEX(m_Mutex);
    auto fid = m_VFS->FindFile(path);
    if (!m_VFS->IsFileValid(fid)) {
        //TODO: todo
        return false;
    }

    auto h = m_VFS->OpenFile(fid, StarVFS::RWMode::W);
    if(!h) {
        //TODO: todo
        return false;
    }

    auto ret = h.SetFileData(data);
    h.Close();

    QueueFileProcessing(uri);

    return ret;
}

bool FileSystem::CreateFile(const std::string & uri) {
    if (uri.empty())
        return false;

    std::string path;
    if (!TranslateURI(uri, path)) {
        return false;
    }

    LOCK_MUTEX(m_Mutex);
    auto h = m_VFS->OpenFile(path, StarVFS::RWMode::RW, StarVFS::OpenMode::CreateNew);

    if (!h) {
        AddLog(Error, "Failed to create file " << uri);
        return false;
    }

    h.Close();

    emit Changed();
    return true;
}

bool FileSystem::CreateDirectory(const std::string & uri) {
    if (uri.empty())
        return false;

    std::string path;
    if (!TranslateURI(uri, path)) {
        return false;
    }

    LOCK_MUTEX(m_Mutex);
    auto h = m_VFS->CreateDirectory(path);

    if (!h) {
        AddLog(Error, "Failed to create directory " << uri);
        return false;
    }
    h.Close();

    emit Changed();
    return true;
}

bool FileSystem::TranslateURI(const std::string & uri, std::string & out) {
    if (uri.empty()) {
        AddLog(Error, "Empty URI!");
        return false;
    }
    auto pos = uri.find("://");
    if (pos == std::string::npos) {
        __debugbreak();
        return false;
    }
    auto start = pos + 3;

    switch (Space::Utils::MakeHash32(uri.c_str(), pos)) {
    case "file"_Hash32:
        out = uri.substr(start);
        return true;
    default:
        AddLog(Error, "Invalid URI: " << uri);
        return false;
    }
}

std::string FileSystem::TranslateToSystem(const std::string & uri) {
    std::string r;
    TranslateURI(uri, r);
    return m_BasePath + r;
}

//-----------------------------------------------------------------------------

void FileSystem::QueueFileProcessing(const std::string & URI) {
    std::string fpath;
    if (!TranslateURI(URI, fpath)) {
        AddLogf(Error, "Invalid file URI: %s", URI.c_str());
        return;
    }

    auto pos = fpath.rfind(".");
    if (pos == std::string::npos) {
        AddLogf(Error, "Cannot find file extension: %s", fpath.c_str());
        return;
    }

    auto ext = fpath.substr(pos + 1);
    auto pit = m_ExtFileProcessorList.find(ext);
    if (pit == m_ExtFileProcessorList.end() || pit->second.empty()) {
    //	AddLogf(Info, "There is no registered processors which can handle file %s(%s)", fpath.c_str(), URI.c_str());
        return;
    }

    for (auto &pinfo : pit->second) {
        auto proc = pinfo.lock();
        if (proc) {
            auto processor = proc->CreateFileProcessor(URI);
            if (!processor) {
                AddLogf(Error, "Failed to create file processor: %s", typeid(*proc.get()).name());
                continue;
            }
            AddLogf(Info, "Queued processing of file %s", URI.c_str());
            processor->SetFence(jobFence);
            jobProcessor->Queue(processor);
            //m_QueuedFileProcessors.push_back(std::move(processor));
        }
    }
}

void FileSystem::HandleDirectoryChanged(const std::string &URI) {
    AddLogf(Info, "Directory changed: %s", URI.c_str());
    changedPaths.insert(URI);
    watcherTimeout.start();
}

void FileSystem::RefreshChangedPaths() {
    AddLogf(Info, "Processing changed paths");
    {
        std::unique_lock<QtShared::iJobFence> fenceLock(*jobFence);

        for (auto &item : changedPaths) {
            if (item.find(m_BasePath) != 0) {
                continue;
            }
            if (boost::filesystem::is_regular_file(item)) {
                auto uri = "file://" + item.substr(m_BasePath.size() - 1);
                QueueFileProcessing(uri);
                continue;
            }
            if (boost::filesystem::is_directory(item)) {
                //TODO
            }
        }
        changedPaths.clear();
        Reload();
    }
}

//-----------------------------------------------------------------------------

void FileSystem::ProjectChanged(Module::SharedDataModule datamod) {
    m_Module = datamod;
    if (m_Module) {
        std::unique_lock<QtShared::iJobFence> fenceLock(*jobFence);
        m_BasePath = m_Module->GetBaseDirectory();
        UpdateFsWatcher();
        Reload();
    }
}

void FileSystem::UpdateFsWatcher() {
    fsWatcher.removePaths(fsWatcher.directories());
    QStringList list;
    QDirIterator it(m_BasePath.c_str(), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto v = it.next();
        std::string fn = QDir(v).dirName().toLocal8Bit().data();
        if (fn != "." && fn != "..") {
            list.push_back(v);
        }
    }
    list.push_back(m_BasePath.c_str());
    fsWatcher.addPaths(list);
}

void FileSystem::Reload() {
    if (jobProcessor->GetQueuedJobCount() > 0) {
        return;
    }

    LOCK_MUTEX(m_Mutex);

    std::unique_lock<QtShared::iJobFence> fenceLock(*jobFence);

    bool processFiles = false;
    if (m_VFS->GetContainerCount() == 0) {

        auto Load = [this](std::string path) {
            auto ret = m_VFS->CreateContainer<StarVFS::Containers::FolderContainer>("/", path);
            if (ret.first != StarVFS::VFSErrorCode::Success) {
                AddLogf(Error, "Failed to load base container. Code: %d", (int)ret.first);
                return false;
            }
            return true;
        };
        processFiles = true;
        Load("./MoonGlare/DataModules/Base/");
        Load("./MoonGlare/DataModules/Debug/");
        Load(m_BasePath);
    } else {
        for (size_t i = 0; i < m_VFS->GetContainerCount(); ++i) {
            auto c = m_VFS->GetContainer(static_cast<::StarVFS::ContainerID>(i + 1));
            if (!c) {
                AddLogf(Error, "Failed to get base container");
            }
            else {
                c->ReloadContainer();
                c->RegisterContent();
            }
        }
    }
    Changed();

    StarVFS::HandleEnumerateFunc processitem;
    auto svfs = GetVFS();
    processitem = [this, &svfs, &processitem](StarVFS::FileID fid) -> bool {
        auto h = svfs->OpenFile(fid);
        if (!h)
            return true;

        if (h.IsDirectory()) {
            h.EnumerateChildren(processitem);
            return true;
        } 

        auto fileuri = StarVFS::MakeFileURI(h.GetFullPath().c_str());
        QueueFileProcessing(fileuri);

        h.Close();

        return true;
    };

    if (processFiles) {
        auto root = svfs->OpenFile("/");
        root.EnumerateChildren(processitem);
        root.Close();
    }
}

//-----------------------------------------------------------------------------

bool FileSystem::EnumerateFolder(const std::string& Path, FileInfoTable &FileTable, bool Recursive) {
    auto ParentFID = m_VFS->FindFile(Path);

    if (!m_VFS->IsFileValid(ParentFID)) {
        AddLogf(Error, "Failed to open '%s' for enumeration", Path.c_str());
        return false;
    }

    FileTable.reserve(1024);//because thats why
    StarVFS::HandleEnumerateFunc svfsfunc;

    svfsfunc = [this, &svfsfunc, &FileTable, Recursive, ParentFID](StarVFS::FileID fid) ->bool {
        if (ParentFID != fid) {
            auto path = m_VFS->GetFilePath(fid, ParentFID);

            FileInfo fi;
            fi.m_IsFolder = m_VFS->IsFileDirectory(fid);
            fi.m_RelativeFileName = m_VFS->GetFilePath(fid, ParentFID);
            fi.m_FileName = m_VFS->GetFileName(fid);
            fi.m_FID = fid;
            FileTable.push_back(fi);
        }

        if ((Recursive || ParentFID == fid) && m_VFS->IsFileDirectory(fid)) {
            auto handle = m_VFS->OpenFile(fid);
            if (!handle.EnumerateChildren(svfsfunc)) {
                handle.Close();
                return true;
            }
            handle.Close();
        }

        return true;
    };

    return svfsfunc(ParentFID);
}

bool FileSystem::FileExists(const std::string &uri) const {
    std::string u = uri;
    auto pos = u.find("://");
    if(pos != std::string::npos)
        u = u.substr(pos + 3);
    auto fid = m_VFS->FindFile(u);
    return m_VFS->IsFileValid(fid);
}

} //namespace Editor 
} //namespace MoonGlare 
