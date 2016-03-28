#ifndef ROOTCLASSH
#define ROOTCLASSH

struct sFlagStruct {
	char *Name;
	unsigned Flag;
	bool Default;
};

class cRootClass : public GabiLib::GabiObject {
	GABI_DECLARE_CLASS(cRootClass, GabiLib::GabiObject);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public: 
	cRootClass();
	virtual ~cRootClass();
	virtual void WriteNameToXML(xml_node Node) const;
	virtual void Describe() const;
	string Info() const;
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	static void WriteFlagPack(xml_node Node, unsigned Flags, const sFlagStruct *FlagPack);
	static void ReadFlagPack(const xml_node Node, unsigned &Flags, const sFlagStruct *FlagPack);
	virtual void InternalInfo(std::ostringstream &buff) const;
};

class NamedObject : public cRootClass {
	GABI_DECLARE_CLASS(NamedObject, cRootClass)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	NamedObject();
	NamedObject(const string& Name);
	~NamedObject();
	const string& GetName() const { return m_Name; }
	const char* GetCharName() const { return GetName().c_str(); }
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	void SetName(const string& name) { m_Name = name; }
private:
	string m_Name;
};

#endif // CROOTCLASS_H_ 
