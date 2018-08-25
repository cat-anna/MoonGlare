#include <pch.h>
#include <nfMoonGlare.h>

#include <StarVFS/core/nStarVFS.h>
#include <Core/Scripts/LuaApi.h>
#include "Core/DataManager.h"

namespace MoonGlare {
namespace FileSystem {

const DataPathsTable DataSubPaths;

DataPathsTable::DataPathsTable() {
#define _set(X) m_table[(unsigned)DataPath::X] = #X
    _set(Fonts);
#undef _set
}

void DataPathsTable::Translate(string& out, const string& in, DataPath origin) const {
    out.clear();
    out.reserve(1024);
    out += '/';
    out += m_table[(unsigned)origin];
    out += '/';
    out += in;
}

void DataPathsTable::Translate(string& out, DataPath origin) const {
    out.clear();
    out.reserve(1024);
    out += '/';
    out += m_table[(unsigned)origin];
}

//-------------------------------------------------------------------------------------------------

MoonGlareFileSystem* MoonGlareFileSystem::s_instance = nullptr;

MoonGlareFileSystem::MoonGlareFileSystem() : m_StarVFSCallback(this) {
    s_instance = this;

    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::FSEvent, "FSEV");
}

MoonGlareFileSystem::~MoonGlareFileSystem() {
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::Initialize() {
    ASSERT(!m_StarVFS);
    m_StarVFS = std::make_unique<StarVFS::StarVFS>();
    m_StarVFS->SetCallback(&m_StarVFSCallback);
    return true;
}

bool MoonGlareFileSystem::Finalize() {
    m_StarVFS->SetCallback(nullptr);
    m_StarVFS.reset();
    return true;
}

//-------------------------------------------------------------------------------------------------

MoonGlareFileSystem::StarVFSCallback::BeforeContainerMountResult MoonGlareFileSystem::BeforeContainerMount(StarVFS::Containers::iContainer *ptr, const StarVFS::String &MountPoint) {
    ASSERT(ptr);
    AddLogf(Debug, "Testing whether container can be mounted:  cid:%d uri:'%s'", ptr->GetContainerID(), ptr->GetContainerURI().c_str());
    //query module
    //TODO: ?	

    return StarVFSCallback::BeforeContainerMountResult::Mount;
}

void MoonGlareFileSystem::AfterContainerMounted(StarVFS::Containers::iContainer *ptr) {
    ASSERT(ptr);
    AddLogf(Info, "Container has been mounted: cid:%d uri:'%s'", ptr->GetContainerID(), ptr->GetContainerURI().c_str());

    if (!GetDataMgr()->InitModule(ptr)) {
        AddLogf(Error, "Unable to import data module");
    }
    else {
        AddLogf(Hint, "Container '%s' has been mounted", ptr->GetContainerURI().c_str());
    }
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::EnumerateFolder(const string& Path, FileInfoTable &FileTable, bool Recursive) {
    ASSERT(m_StarVFS);

    auto ParentFID = m_StarVFS->FindFile(Path);

    if (!m_StarVFS->IsFileValid(ParentFID)) {
        AddLogf(Error, "Failed to open '%s' for enumeration", Path.c_str());
        return false;
    }

    FileTable.reserve(1024);//because thats why
    StarVFS::HandleEnumerateFunc svfsfunc;

    svfsfunc = [this, &svfsfunc, &FileTable, Recursive, ParentFID](StarVFS::FileID fid) ->bool {
        if (ParentFID != fid) {
            auto path = m_StarVFS->GetFilePath(fid, ParentFID);

            FileInfo fi;
            fi.m_IsFolder = m_StarVFS->IsFileDirectory(fid);
            fi.m_RelativeFileName = m_StarVFS->GetFilePath(fid, ParentFID);
            fi.m_FileName = m_StarVFS->GetFileName(fid);
            fi.m_FID = fid;
            FileTable.push_back(fi);
        }

        if ((Recursive || ParentFID == fid) && m_StarVFS->IsFileDirectory(fid)) {
            auto handle = m_StarVFS->OpenFile(fid);
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

bool MoonGlareFileSystem::EnumerateFolder(DataPath origin, FileInfoTable &FileTable, bool Recursive) {
    std::string path;
    DataSubPaths.Translate(path, origin);
    return EnumerateFolder(path, FileTable, Recursive);
}

bool MoonGlareFileSystem::EnumerateFolder(const string& SubPath, DataPath origin, FileInfoTable &FileTable, bool Recursive) {
    std::string path;
    DataSubPaths.Translate(path, SubPath, origin);
    return EnumerateFolder(path, FileTable, Recursive);
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::OpenFile(StarVFS::ByteTable & FileData, StarVFS::FileID fid) {
    FileData.reset();

    if (!m_StarVFS->IsFileValid(fid)) {
        AddLogf(Warning, "Failed to find file: fid:%u", (unsigned)fid);
        return false;
    }
    if (m_StarVFS->IsFileDirectory(fid)) {
        AddLogf(Error, "Attempt to read directory: fid:%u", (unsigned)fid);
        return false;
    }
    if (!m_StarVFS->GetFileData(fid, FileData)) {
        AddLogf(Warning, "Failed to read file fid:%u", (unsigned)fid);
        return false;
    }

    AddLogf(FSEvent, "Opened file: fid:%u -> %s", (unsigned)fid, m_StarVFS->GetFullFilePath(fid).c_str());
    return true;
}

bool MoonGlareFileSystem::OpenFile(const string& FileName, DataPath origin, StarVFS::ByteTable &FileData) {
    ASSERT(m_StarVFS);

    std::string path;

    if (!TranslateFileName(FileName, path, origin))
        return false;

    auto fid = m_StarVFS->FindFile(path.c_str());
    return OpenFile(FileData, fid);
}

bool MoonGlareFileSystem::TranslateFileName(const std::string & FileName, std::string &path, DataPath origin) {
    auto pos = FileName.find("://");
    if (pos != std::string::npos) {
        auto hash = Space::Utils::MakeHash32(FileName.c_str(), pos);
        pos += 3; //compensate '://'
        switch (hash) {
        case "file"_Hash32:
            path = FileName.substr(pos);
            break;
            //TODO: fid://value
            //TODO: hash://hex
        default:
            AddLogf(Error, "Unknown uri protocol: %s", FileName.c_str());
            DataSubPaths.Translate(path, FileName, origin);
        }
    }
    else {
        if (origin == DataPath::URI) {
            AddLogf(Error, "Invalid uri: %s", FileName.c_str());
            return false;
        }
        DataSubPaths.Translate(path, FileName, origin);
    }

    return true;
}

bool MoonGlareFileSystem::OpenXML(XMLFile &doc, StarVFS::FileID fid) {
    ASSERT(m_StarVFS);

    doc.reset();
    StarVFS::ByteTable data;
    if (!OpenFile(data, fid)) {
        //already logged, no need for more
        return false;
    }

    doc = std::make_unique<pugi::xml_document>();
    auto result = doc->load_string((char*)data.get());

    return static_cast<bool>(result);
}

bool MoonGlareFileSystem::OpenResourceXML(XMLFile &doc, const string& Name, DataPath origin) {
    if (Name.find("://") != std::string::npos)
        return OpenXML(doc, Name);

    string buffer;
    buffer.reserve(128);
    switch (origin) {
    case DataPath::Fonts:
    default:
        buffer += Name;
        buffer += ".xml";
        break;
    }
    return OpenXML(doc, buffer, origin);
}

bool MoonGlareFileSystem::OpenXML(XMLFile &doc, const string& FileName, DataPath origin) {
    ASSERT(m_StarVFS);

    doc.reset();
    StarVFS::ByteTable data;
    if (!OpenFile(FileName, origin, data)) {
        //already logged, no need for more
        return false;
    }

    doc = std::make_unique<pugi::xml_document>();
    auto result = doc->load_string((char*)data.get());

    return static_cast<bool>(result);
}

//----------------------------------------------------------------------------------

void MoonGlareFileSystem::DumpStructure(std::ostream &out) const {
    ASSERT(m_StarVFS);
    out << "STRUCTURE:\n";
    m_StarVFS->DumpStructure(out);
    out << "\n\n";
    out << "FILE TABLE:\n";
    m_StarVFS->DumpFileTable(out);
    out << "\n\n";
    out << "HASH TABLE:\n";
    m_StarVFS->DumpHashTable(out);
}

bool MoonGlareFileSystem::LoadContainer(const std::string &URI) {
    ASSERT(m_StarVFS);

    auto ret = m_StarVFS->OpenContainer(URI);
    if (ret != StarVFS::VFSErrorCode::Success) {
        AddLogf(Error, "Failed to open container '%s' code:%d", URI.c_str(), (int)ret);
        return false;
    }

    return true;
}

void MoonGlareFileSystem::FindFiles(const char * ext, StarVFS::DynamicFIDTable & out) {
    ASSERT(m_StarVFS);
    m_StarVFS->FindFilesByTypeHash((StarVFS::CString)ext, out);
}

const char * MoonGlareFileSystem::GetFileName(StarVFS::FileID fid) const {
    ASSERT(m_StarVFS);
    return m_StarVFS->GetFileName(fid);
}

std::string MoonGlareFileSystem::GetFullFileName(StarVFS::FileID fid) const {
    ASSERT(m_StarVFS);
    return m_StarVFS->GetFullFilePath(fid);
}

} //namespace FileSystem 
} //namespace MoonGlare 
