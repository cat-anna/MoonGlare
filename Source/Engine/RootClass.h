#pragma once

class cRootClass : public Space::RTTI::RTTIObject {
	SPACERTTI_DECLARE_CLASS(cRootClass, Space::RTTI::RTTIObject);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public: 
	virtual void WriteNameToXML(xml_node Node) const;
	static void RegisterScriptApi(ApiInitializer &api);
};

class NamedObject : public cRootClass {
	SPACERTTI_DECLARE_CLASS(NamedObject, cRootClass)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	NamedObject();
	NamedObject(const string& Name);

	const string& GetName() const { return m_Name; }
	const char* GetCharName() const { return GetName().c_str(); }
	static void RegisterScriptApi(ApiInitializer &api);
	void SetName(const string& name) { m_Name = name; }
private:
	string m_Name;
};
