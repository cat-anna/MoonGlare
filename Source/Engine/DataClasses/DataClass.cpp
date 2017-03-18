#include <pch.h>
#include <MoonGlare.h>

namespace MoonGlare {
namespace DataClasses {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(BasicResource)
RegisterApiDerivedClass(BasicResource, &BasicResource::RegisterScriptApi);

BasicResource::BasicResource() {
}

BasicResource::BasicResource(const string& Name): 
		BaseClass(Name) { 
}

void BasicResource::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cBasicResource")
		//.addFunction("GetDataReader", &ThisClass::GetDataReader)
	.endClass()
	;
}

DataPath BasicResource::GetResourceType() const {
	LOG_ABSTRACT_FUNCTION();
	CriticalError("Abstract function called!");
}

FileSystem::XMLFile BasicResource::OpenMetaData() const {
	FileSystem::XMLFile xml;
	if (!GetFileSystem()->OpenResourceXML(xml, GetName(), GetResourceType())) {
		AddLogf(Error, "Unable to open master resource xml for resource '%s' of class '%s'", GetName().c_str(), GetDynamicTypeInfo()->GetName());
		return nullptr;
	}
	return xml;
}

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(DataClass)
RegisterApiDerivedClass(DataClass, &DataClass::RegisterScriptApi);

DataClass::DataClass(): 
		m_Flags(0) {
}

DataClass::DataClass(const string& Name): 
		BaseClass(Name), 
		m_Flags(0) { 
}

DataClass::~DataClass() {
	AddLogf(Debug, "Destroying resource '%s'", GetName().c_str());
}

//---------------------------------------------------------------------------------------------------

void DataClass::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cDataClass")
	.endClass()
	;
}

bool DataClass::Initialize() {
	if (IsReady())
		return true;

	if (!DoInitialize()) {
		AddLogf(Error, "Unable to initialize resource '%s' of class '%s'", GetName().c_str(), GetDynamicTypeInfo()->GetName());
		return false;
	}

	SetReady(true);
	AddLogf(Debug, "Initialized resource '%s' of class '%s'", GetName().c_str(), GetDynamicTypeInfo()->GetName());
	return true;
}

bool DataClass::Finalize() {
	if (!IsReady())
		return true;

	SetReady(false);

	if (!DoFinalize()) {
		AddLogf(Error, "Unable to initialize resource '%s' of class '%s'", GetName().c_str(), GetDynamicTypeInfo()->GetName());
		return false;
	}
	AddLogf(Debug, "Finalized resource resource '%s' of class '%s'", GetName().c_str(), GetDynamicTypeInfo()->GetName());
	return true;
}

//---------------------------------------------------------------------------------------------------

bool DataClass::DoInitialize() {
	return true;
}

bool DataClass::DoFinalize() {
	return true;
}

//---------------------------------------------------------------------------------------------------

bool DataClass::ReadNameFromXML(const xml_node Node) {
	SetName(Node.attribute("Name").as_string(""));
	if (GetName().empty()) {
		AddLog(Debug, "Resource definition without name!");
		return false;
	}
	return true;
}

} // namespace DataClasses
} //namespace MoonGlare 
