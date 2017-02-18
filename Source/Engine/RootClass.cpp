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
