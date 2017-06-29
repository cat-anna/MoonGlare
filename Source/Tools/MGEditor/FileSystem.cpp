/*
  * Generated by cppsrc.sh
  * On 2016-07-22 11:09:18,36
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "FileSystem.h"

#include <StarVFS/core/Container/FolderContainer.h>
#include "Module/DataModule.h"
#include "AsyncFileProcessor.h"

namespace MoonGlare {
namespace Editor {

FileSystem::FileSystem(QtShared::SharedModuleManager modmgr) :
        moduleManager(std::move(modmgr)) {

    m_VFS = std::make_shared<StarVFS::StarVFS>();
    connect(Notifications::Get(), SIGNAL(ProjectChanged(Module::SharedDataModule)), this, SLOT(ProjectChanged(Module::SharedDataModule)));

    m_AsyncFileProcessor = std::make_unique<AsyncFileProcessor>();
    connect(this, &FileSystem::FileProcessorCreated, m_AsyncFileProcessor.get(), &AsyncFileProcessor::FileProcessorCreated);

    for (auto mod : moduleManager->QuerryInterfaces< QtShared::iFileProcessorInfo>()) {
        for (auto &it : mod.m_Interface->GetSupportedTypes()) {
            m_ExtFileProcessorList[it].push_back(mod.m_Interface);
            AddLogf(Info, "Registered file processor %s -> '%s'", it.c_str(), typeid(*mod.m_Module).name());
        }
    }
}

FileSystem::~FileSystem() {
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
            emit FileProcessorCreated(processor);
            //m_QueuedFileProcessors.push_back(std::move(processor));
        }
    }
}

//-----------------------------------------------------------------------------

void FileSystem::ProjectChanged(Module::SharedDataModule datamod) {
    m_Module = datamod;
    if (m_Module) {
        m_BasePath = m_Module->GetBaseDirectory();
        Reload();
    }
}

void FileSystem::Reload() {
    LOCK_MUTEX(m_Mutex);
    if (m_VFS->GetContainerCount() == 0) {
        auto ret = m_VFS->CreateContainer<StarVFS::Containers::FolderContainer>("/", m_BasePath);
        if (ret.first != StarVFS::VFSErrorCode::Success) {
            AddLogf(Error, "Failed to load base container. Code: %d", (int)ret.first);
            return;
        }
    } else {
        auto c = m_VFS->GetContainer(1);
        if (!c) {
            AddLogf(Error, "Failed to get base container");
            return;
        }
        c->ReloadContainer();
        c->RegisterContent();
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

    auto root = svfs->OpenFile("/");
    root.EnumerateChildren(processitem);
    root.Close();
}

} //namespace Editor 
} //namespace MoonGlare 
