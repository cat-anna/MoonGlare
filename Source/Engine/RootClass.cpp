#include <pch.h>
#include <nfMoonGlare.h>

SPACERTTI_IMPLEMENT_CLASS(cRootClass);
RegisterApiBaseClass(cRootClass, &cRootClass::RegisterScriptApi);

void cRootClass::RegisterScriptApi(ApiInitializer &api) {
	api.beginClass<ThisClass>("cRootClass")
#ifndef _DISABLE_SCRIPT_ENGINE_
		.addCFunction("ExactClass", &ThisClass::PushExactClass)
#endif
	.endClass();
}

void cRootClass::WriteNameToXML(xml_node Node) const {
	Node.append_attribute("Class") = GetDynamicTypeInfo()->GetName();
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

void NamedObject::RegisterScriptApi(ApiInitializer &api) {
	api.deriveClass<ThisClass, BaseClass>("cNamedObject")
		.addFunction("GetName", &NamedObject::GetCharName)
	.endClass();
}
