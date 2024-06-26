#pragma once

#include <Foundation/iFileSystem.h>
#include <Foundation/Scripts/ApiInit.h>

enum class DataPath {
    Fonts,
    URI,

    MaxValue,
};

namespace MoonGlare {
namespace FileSystem {

//-------------------------------------------------------------------------------------------------

/** File system is not yes fully thread-safe! */
class MoonGlareFileSystem : public iFileSystem {
public:
    static MoonGlareFileSystem* s_instance;

    MoonGlareFileSystem(InterfaceMap &ifaceMap);
    virtual ~MoonGlareFileSystem();

 //iFileSystem
    bool OpenFile(StarVFS::ByteTable &FileData, const std::string& uri) override { return OpenFile(uri, DataPath::URI, FileData);  }
    bool OpenFile(StarVFS::ByteTable &FileData, StarVFS::FileID fid) override;
    bool EnumerateFolder(const std::string& Path, FileInfoTable &FileTable, bool Recursive);
    void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out) override;
    std::string GetFileName(StarVFS::FileID fid) const override;
    std::string GetFullFileName(StarVFS::FileID fid) const override;
    bool FileExists(const std::string &uri) const override;


    bool LoadContainer(const std::string &URI);

    bool TranslateFileName(const std::string & FileName, std::string &path, DataPath origin);

    bool OpenFile(const std::string& FileName, DataPath origin, StarVFS::ByteTable &FileData);
    bool OpenXML(XMLFile &doc, StarVFS::FileID fid);
    bool OpenXML(XMLFile &doc, const std::string& FileName, DataPath origin);
    bool OpenResourceXML(XMLFile &doc, const std::string& Name, DataPath origin = DataPath::URI);
    
    bool OpenXML(XMLFile &doc, std::string ResName, const std::string& FileName, DataPath origin) {
        return OpenXML(doc, (ResName += '/') += FileName, origin);
    }
    bool OpenFile(std::string ResName, const std::string& FileName, DataPath origin, StarVFS::ByteTable &FileData) {
        return OpenFile((ResName += '/') += FileName, origin, FileData);
    }

    bool EnumerateFolder(DataPath origin, FileInfoTable &FileTable, bool Recursive);
    bool EnumerateFolder(const std::string& SubPath, DataPath origin, FileInfoTable &FileTable, bool Recursive);

    void DumpStructure(std::ostream &out) const;
private:
    struct StarVFSCallback : public StarVFS::StarVFSCallback {
        virtual BeforeContainerMountResult BeforeContainerMount(StarVFS::Containers::iContainer *ptr, const StarVFS::String &MountPoint) override { return m_Owner->BeforeContainerMount(ptr, MountPoint); }
        virtual void AfterContainerMounted(StarVFS::Containers::iContainer *ptr) override { return m_Owner->AfterContainerMounted(ptr); }
        StarVFSCallback(MoonGlareFileSystem *Owner) : m_Owner(Owner) {}
        MoonGlareFileSystem *m_Owner;
    };

    StarVFSCallback::BeforeContainerMountResult BeforeContainerMount(StarVFS::Containers::iContainer *ptr, const StarVFS::String &MountPoint);
    void AfterContainerMounted(StarVFS::Containers::iContainer *ptr);

    InterfaceMap &interfaceMap;
    StarVFSCallback m_StarVFSCallback;
    std::unique_ptr<StarVFS::StarVFS> m_StarVFS;
};

struct DataPathsTable {
    const std::string& operator[](DataPath p) const { return m_table[(unsigned)p]; }
    DataPathsTable();
    void Translate(std::string& out, const std::string& in, DataPath origin) const;
    void Translate(std::string& out, DataPath origin) const;
private:
    std::string m_table[(unsigned)DataPath::MaxValue];
};
extern const DataPathsTable DataSubPaths;

} //namespace FileSystem 

inline FileSystem::MoonGlareFileSystem* GetFileSystem() { return FileSystem::MoonGlareFileSystem::s_instance; }

} //namespace MoonGlare 

using MoonGlare::XMLFile;
