/*
  * Generated by cppsrc.sh
  * On 2015-02-24 13:06:08,58
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FileSystem_H
#define FileSystem_H

enum class DataPath {
	Root,

	Models,
	Fonts,
	Shaders,
	Scripts,
	Sounds,
	Music,
	Texture,
	XML,
	Tables,
	Objects,

	URI,

	MaxValue,
};

#ifdef _BUILDING_ASSETS_
//ugly hack
namespace Graphic {
	struct Texture;
}
#endif

namespace MoonGlare {
namespace FileSystem {

using XMLFile = std::unique_ptr<pugi::xml_document>;
using TextureFile = std::unique_ptr < Graphic::Texture > ;

struct FileInfo {
	std::string m_FileName;
	std::string m_RelativeFileName;
	StarVFS::FileID m_FID;
	bool m_IsFolder;
};
using FileInfoTable = std::vector<FileInfo>;

//-------------------------------------------------------------------------------------------------

/** File system is not yes fully thread-safe! */
class MoonGlareFileSystem : public Space::RTTI::RTTIObject {
	SPACERTTI_DECLARE_CLASS_SINGLETON(MoonGlareFileSystem, Space::RTTI::RTTIObject)
public:
 	MoonGlareFileSystem();
 	virtual ~MoonGlareFileSystem();

	bool LoadContainer(const std::string &URI);

	void FindFiles(const char *ext, StarVFS::DynamicFIDTable &out);

	const char *GetFileName(StarVFS::FileID fid) const;
	std::string GetFullFileName(StarVFS::FileID fid) const;

	bool TranslateFileName(const std::string & FileName, std::string &path, DataPath origin);

	/** Read raw file data */
	bool OpenFile(StarVFS::ByteTable &FileData, StarVFS::FileID fid);
	/** Read raw file data */
	bool OpenFile(const string& FileName, DataPath origin, StarVFS::ByteTable &FileData);
	/** Open an xml document */
	bool OpenXML(XMLFile &doc, StarVFS::FileID fid);
	/** Open an xml document */
	bool OpenXML(XMLFile &doc, const string& FileName, DataPath origin = DataPath::URI);
	/** Open resource xml document in fmt: 'origin/NAME/NAME.xml' [depends on resource type] */
	bool OpenResourceXML(XMLFile &doc, const string& Name, DataPath origin = DataPath::URI);
	
	bool OpenXML(XMLFile &doc, string ResName, const string& FileName, DataPath origin = DataPath::URI) {
		return OpenXML(doc, (ResName += '/') += FileName, origin);
	}
	bool OpenFile(string ResName, const string& FileName, DataPath origin, StarVFS::ByteTable &FileData) {
		return OpenFile((ResName += '/') += FileName, origin, FileData);
	}

	bool Initialize();
	bool Finalize();

	bool EnumerateFolder(const string& Path, FileInfoTable &FileTable, bool Recursive);
	bool EnumerateFolder(DataPath origin, FileInfoTable &FileTable, bool Recursive);
	bool EnumerateFolder(const string& SubPath, DataPath origin, FileInfoTable &FileTable, bool Recursive);

	static void RegisterDebugScriptApi(ApiInitializer &api);
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

	StarVFSCallback m_StarVFSCallback;
	std::unique_ptr<StarVFS::StarVFS> m_StarVFS;
};

#ifndef _BUILDING_ASSETS_

class DirectoryReader : public cRootClass {
	SPACERTTI_DECLARE_STATIC_CLASS(DirectoryReader, cRootClass);
public:
	DirectoryReader() : m_origin(DataPath::Root), m_OwnerName("") { }
	DirectoryReader(DataPath origin, const string& OwnerName = "") : m_origin(origin), m_OwnerName(OwnerName) { }

	bool OpenXML(XMLFile &xml, const string& FileName) {
		if (m_OwnerName.empty())
			return MoonGlareFileSystem::Instance()->OpenXML(xml, FileName, m_origin);
		else
			return MoonGlareFileSystem::Instance()->OpenXML(xml, (m_OwnerName + '/') += FileName, m_origin);
	}
	bool OpenFile(const string& FileName, StarVFS::ByteTable &FileData) {
		if (m_OwnerName.empty())
			return MoonGlareFileSystem::Instance()->OpenFile(FileName, m_origin, FileData);
		else
			return MoonGlareFileSystem::Instance()->OpenFile((m_OwnerName + '/') += FileName, m_origin, FileData);
	}
	
	std::string translate(const string& FileName) {
		std::string out;
		if (m_OwnerName.empty())
			MoonGlareFileSystem::Instance()->TranslateFileName(FileName, out, m_origin);
		else
			MoonGlareFileSystem::Instance()->TranslateFileName((m_OwnerName + '/') += FileName, out, m_origin);
		return out;
	}
protected:
	DataPath m_origin;
	string m_OwnerName;
};

#endif

struct DataPathsTable {
	const string& operator[](DataPath p) const { return m_table[(unsigned)p]; }
	DataPathsTable();
	void Translate(string& out, const string& in, DataPath origin) const;
	void Translate(string& out, DataPath origin) const;
private:
	string m_table[(unsigned)DataPath::MaxValue];
};
extern const DataPathsTable DataSubPaths;

} //namespace FileSystem 

inline FileSystem::MoonGlareFileSystem* GetFileSystem() { return FileSystem::MoonGlareFileSystem::Instance(); }

} //namespace MoonGlare 

using MoonGlare::FileSystem::XMLFile;

#endif
