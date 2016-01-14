/*
  * Generated by cppsrc.sh
  * On 2015-02-26  2:51:49,93
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef InternalContainer_H
#define InternalContainer_H

namespace MoonGlare {
namespace FileSystem {

struct InternalFileFlags {
	enum {
		Valid		= 0x01,
		HasChildren = 0x02,

		Folder		= Valid | HasChildren,
		File		= Valid,
	};
};

struct InternalFileNode : public RawFilePointer {
	unsigned Flags = 0;
	const void *Data = nullptr;
	unsigned Size = 0;
	const InternalFileNode* Children = nullptr;
	const char *Name = 0;

//{Flags::Folder, 0, 0, Root_folder, "Internal", },

	InternalFileNode() { }
	InternalFileNode(unsigned flag, const void *data, unsigned size, const InternalFileNode *children, const char* name):
		Flags(flag), Data(data), Size(size), Children(children), Name(name) { }

	virtual FileSize GetFileSize() const override { return Size; };
};

class InternalContainer : public iContainer {
	GABI_DECLARE_STATIC_CLASS(InternalContainer, iContainer)
public:

	using FileFlags = InternalFileFlags;
	using FileNode = InternalFileNode;
	
 	InternalContainer(const InternalFileNode *Root);
 	virtual ~InternalContainer();

	virtual FileReader GetFileReader(const string& file) const override;
	virtual FileReader GetFileReader(const RawFilePointer *file) const override;
	virtual bool FileExists(const string& file) const override;
	virtual bool EnumerateFolder(const RawFilePointer *root, FolderEnumerateFunc func) const override;
	virtual void DumpFileTree(ostream& out) const override;

	virtual const string& GetFileName() const override;
	virtual const string& GetContainerName() const override;
protected:
	const FileNode *m_Root;
	const FileNode* LookForFile(const string& FileName) const;
	string m_Name;
};

} //namespace FileSystem 
} //namespace MoonGlare 

#endif
