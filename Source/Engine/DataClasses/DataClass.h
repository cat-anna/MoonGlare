#ifndef DataClassH
#define DataClassH

namespace DataClasses {

class BasicResource : public NamedObject {
	GABI_DECLARE_CLASS_NOCREATOR(BasicResource, NamedObject)
public:
	BasicResource(DataModule *Owner = 0);
	BasicResource(const string& Name);
	DataModule* GetDataModule() { return m_Owner; }
	DataModule* GetDataModule() const { return m_Owner; }
	static void RegisterScriptApi(ApiInitializer &api);

	FileSystem::DirectoryReader GetDataReader() const { return FileSystem::DirectoryReader(GetResourceType(), GetName()); }
	virtual DataPath GetResourceType() const;
protected:
	void SetDataModule(DataModule *Owner);
	FileSystem::XMLFile OpenMetaData() const;
private:
	DataModule *m_Owner;
};

/** basic class for data containers like font */
class DataClass : public BasicResource {
	GABI_DECLARE_ABSTRACT_CLASS(DataClass, BasicResource)
public:
	virtual ~DataClass();
	DataClass(DataModule *Owner = 0);
	DataClass(const string& Name);

	virtual bool Initialize();
	virtual bool Finalize();

	DefineFlagGetter(m_Flags, Flags::Ready, Ready);
	DefineFlagGetter(m_Flags, Flags::Loaded, Loaded);

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	unsigned m_Flags;

	virtual void WriteNameToXML(xml_node Node) const;
	virtual bool ReadNameFromXML(const xml_node Node);

	virtual bool DoInitialize();
	virtual bool DoFinalize();
	
	struct Flags {
		enum {
			Ready		= 0x0001,
			Loaded		= 0x0002,
		};
	};

	DefineFlagSetter(m_Flags, Flags::Ready, Ready);
	DefineFlagSetter(m_Flags, Flags::Loaded, Loaded);

	virtual void InternalInfo(std::ostringstream &buff) const;
};

} // namespace DataClasses

#endif
