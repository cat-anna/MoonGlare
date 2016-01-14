/*
  * Generated by cppsrc.sh
  * On 2015-02-26  2:08:08,26
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FolderContainer_H
#define FolderContainer_H

#ifdef _FEATURE_FOLDER_CONTAINER_SUPPORT_

namespace MoonGlare {
namespace FileSystem {

struct FolderContainerPointer;

class FolderContainer : public iContainer {
	GABI_DECLARE_CLASS_NOCREATOR(FolderContainer, iContainer)
public:
 	FolderContainer(const string &Path);
 	virtual ~FolderContainer();

	virtual FileReader GetFileReader(const string& file) const override;
	virtual FileReader GetFileReader(const RawFilePointer *file) const override;
	virtual FileWritter GetFileWritter(const string& file) override;
	virtual FileWritter GetFileWritter(const RawFilePointer *filee) override;

	virtual bool FileExists(const string& file) const override;

	virtual const string& GetFileName() const override;

	virtual bool EnumerateFolder(const RawFilePointer *root, FolderEnumerateFunc func) const override;
private: 
	string m_Path;

	string FullPath(const string &sub) const { return m_Path + sub; }

	mutable std::list<std::unique_ptr<FolderContainerPointer>> m_PtrList;
	mutable std::unordered_map<string, FolderContainerPointer*> m_PtrMap;
};

} //namespace FileSystem 
} //namespace MoonGlare 

#endif
#endif
