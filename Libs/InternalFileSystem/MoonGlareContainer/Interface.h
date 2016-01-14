#ifndef MODULECONTINERINTERFACEH
#define MODULECONTINERINTERFACEH

namespace Container {

enum {
	mcDontUsePathHash			= 0x00001000,
	mcIsReady					= 0x00000001,
	mcReadable					= 0x00000002,
	mcWritable					= 0x00000004,
	
	mcFolder					= 0x10000000,
	
	mcVersion_0					= 1,
	
	mcVersion_Lastest = mcVersion_0,


	reader_gotWholeData				= 0x00100000,
};

class ModuleContainer;

class cFileNode {
public:
	typedef std::list<cFileNode*> FileList;
	cFileNode *Parent;
	FileList Children;
	unsigned Flags;
	string Name;
	unsigned Size;
	unsigned PathHash;
	cFileNode * AppendChild(cFileNode *n);
	virtual ~cFileNode();
};

class cContainerFile :  public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(cContainerFile, cRootClass);
protected:
	const ModuleContainer *m_Owner;
	unsigned m_Flags;	
public:
	cContainerFile(const ModuleContainer *Owner) : cRootClass(), m_Owner(Owner), m_Flags(0) {}
	virtual ~cContainerFile() { }
	virtual void done() {
		//send signal to reader that is no needed anymore. TODO
		delete this;
	}	
};

class cFileWritter : public cContainerFile {
	GABI_DECLARE_CLASS_NOCREATOR(cFileWritter, cContainerFile);
protected:
	std::stringstream ss;
public:
	cFileWritter(const ModuleContainer *Owner): cContainerFile(Owner) { }
	
	virtual std::ostream& as_stream() { return ss; }
};

class cFileReader : public cContainerFile {
	GABI_DECLARE_CLASS_NOCREATOR(cFileReader, cContainerFile);
protected:
	cFileNode *m_File;
public:
	cFileReader(const ModuleContainer *Owner, cFileNode *File): cContainerFile(Owner), m_File(File) { }
	virtual ~cFileReader() { }

	virtual const char* GetFileData() = 0;
	virtual unsigned size() const { return m_File->Size; }
	virtual const string GetFileName() const { return m_File->Name; }
//	virtual unsigned read(char* dest, unsigned count) = 0;
};

class ModuleContainer : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(ModuleContainer, cRootClass);
protected:
	unsigned m_Flags;
	unsigned m_Version;
	cFileNode *m_RootNode;	
	virtual cFileNode* FindFile(const string& FileName) const;
public:
	ModuleContainer();
	virtual ~ModuleContainer();
	virtual void DumpTree(std::ostream& out) const;
	
	bool IsReady() const { return (m_Flags & mcIsReady) > 0; };
	bool IsReadOnly() const { return (m_Flags & mcReadable) > 0; };
	bool IsWritable() const { return (m_Flags & mcWritable) > 0; };

	bool IsOk() const { return true; }//TODO!!!
	
	virtual cFileReader* GetFile(const string& File) const = 0;
	virtual cFileWritter* GetFileWritter(const string &file);
	
	virtual bool FileExists(const string& File) const;
	virtual bool DeleteFile(const string& File);
	virtual unsigned FilesCount() const;

	virtual bool EnumerateFolder(const string &Path, std::function<void(const string&)> func) const;
};

} //namespace container

#endif
