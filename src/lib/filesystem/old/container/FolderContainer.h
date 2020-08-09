/*
  * Generated by cppsrc.sh
  * On 2015-12-10 19:39:52,96
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FolderContainer_H
#define FolderContainer_H
#ifndef STARVFS_DISABLE_FOLDERCONTAINER
namespace StarVFS {
namespace Containers {

class FolderContainer : public iContainer {
public:
	FolderContainer(FileTableInterface *fti, String Path = ".");
	virtual ~FolderContainer();

	virtual bool ReloadContainer() override;
	virtual bool RegisterContent() const override;

	virtual FileID GetFileCount() const override;

	virtual String GetContainerURI() const override;

	virtual RWMode GetRWMode() const override;

	virtual bool GetFileData(FileID ContainerFID, ByteTable &out) const override;
	virtual bool SetFileData(FileID ContainerFID, const ByteTable &in) const override;
	virtual FileID FindFile(const String& ContainerFileName) const override;
	virtual bool EnumerateFiles(ContainerFileEnumFunc filterFunc)const override;
	virtual bool CreateFile(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr) override;
	virtual bool CreateDirectory(FileID ContainerBaseFID, ConstCString Name, FileID *GlobalFIDOutput = nullptr) override;

    bool GetFileSystemPath(FileID cfid, std::string &out);

	static bool CanOpen(const String& Location);
	static CreateContainerResult CreateFor(StarVFS *svfs, const String& MountPoint, const String& Location);
private:
	enum class FileType {
		File, Directory,
	};

	struct Entry {
		FileType m_Type;
		FileID m_GlobalFid;
		String m_FullPath;
		String m_SubPath;
		uint64_t m_FileSize;
	};

	template <class T> static bool EnumerateFolder(const String &Path, T func);

	String m_Path;

	std::vector<Entry> m_FileEntry;

	bool ScanPath();
	bool AllocateFile(FileID ContainerBaseFID, ConstCString Name, Entry *&out, size_t &index);
};

} //namespace Containers 
} //namespace StarVFS 

#endif
#endif
