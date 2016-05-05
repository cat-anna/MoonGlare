#include <pch.h>
#include <nfMoonGlare.h>

SPACERTTI_IMPLEMENT_CLASS(cRootClass);
RegisterApiBaseClass(cRootClass, &cRootClass::RegisterScriptApi);

cRootClass::cRootClass(): 
		BaseClass() {
}

cRootClass::~cRootClass() {
}

void cRootClass::RegisterScriptApi(ApiInitializer &api) {
	api.beginClass<ThisClass>("cRootClass")
#ifndef _DISABLE_SCRIPT_ENGINE_
		.addCFunction("ExactClass", &ThisClass::PushExactClass)
#endif
		.addFunction("Describe", &ThisClass::Describe)
	.endClass();
}

void cRootClass::Describe() const {
	char buff[1024] = {};
	sprintf(buff, "%s@0x%08x", GetDynamicTypeInfo()->Name, (unsigned)this);
	AddLog(Hint, buff << " [" << Info() << "]");
}

void cRootClass::InternalInfo(std::ostringstream &buff) const {
	if(buff.tellp() > 0) buff << " ";
	buff << "Class:'" << GetDynamicTypeInfo()->Name << "'";
}

void cRootClass::WriteNameToXML(xml_node Node) const {
	Node.append_attribute("Class") = GetDynamicTypeInfo()->Name;
}

string cRootClass::Info() const {
	std::ostringstream buff;
	InternalInfo(buff);
	return buff.str();
}

void cRootClass::WriteFlagPack(xml_node Node, unsigned Flags, const sFlagStruct *FlagPack) {
	for(const sFlagStruct *i = FlagPack; i->Name; ++i){
		bool f = Flags & i->Flag;
		if(f == i->Default) continue;
		Node.append_attribute(i->Name) = f;
	}
}

void cRootClass::ReadFlagPack(const xml_node Node, unsigned &Flags, const sFlagStruct *FlagPack) {
	for(const sFlagStruct *i = FlagPack; i->Name; ++i){
		bool f = Node.attribute(i->Name).as_bool(i->Default);
		if(f)
			Flags |= i->Flag;
		else
			Flags &= ~i->Flag;
	}
}

//----------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS(NamedObject);
RegisterApiDerivedClass(NamedObject, &NamedObject::RegisterScriptApi);

NamedObject::NamedObject(): cRootClass(), m_Name("?") { }
NamedObject::NamedObject(const string& Name): cRootClass(), m_Name(Name) { }
NamedObject::~NamedObject() { }

void NamedObject::RegisterScriptApi(ApiInitializer &api) {
	api.deriveClass<ThisClass, BaseClass>("cNamedObject")
		.addFunction("GetName", &NamedObject::GetCharName)
	.endClass();
}
