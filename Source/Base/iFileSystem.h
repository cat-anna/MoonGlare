#pragma once

namespace MoonGlare {

struct FileInfo {
    std::string m_FileName;
    std::string m_RelativeFileName;
    StarVFS::FileID m_FID;
    bool m_IsFolder;
};
using FileInfoTable = std::vector<FileInfo>;

using XMLFile = std::unique_ptr<pugi::xml_document>;

class iFileSystem {
protected:
    virtual ~iFileSystem() {};
public:
//    virtual bool OpenFile(StarVFS::ByteTable &FileData, StarVFS::FileID fid) = 0;
    virtual bool OpenFile(StarVFS::ByteTable &FileData, const std::string& uri) = 0;

    virtual bool OpenXML(XMLFile &doc, const std::string& uri) = 0;
    virtual bool EnumerateFolder(const std::string& Path, FileInfoTable &FileTable, bool Recursive) = 0;
};

}