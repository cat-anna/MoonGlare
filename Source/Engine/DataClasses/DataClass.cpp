#include <pch.h>
#include <nfMoonGlare.h>

#include <Foundation/CriticalException.h>
#include <Core/Scripts/LuaApi.h>

namespace MoonGlare {
namespace DataClasses {

RegisterApiBaseClass(BasicResource, &BasicResource::RegisterScriptApi);

BasicResource::BasicResource() {
}

BasicResource::BasicResource(const string& Name): m_Name(Name) { 
}

void BasicResource::RegisterScriptApi(ApiInitializer &api) {
	api
	.beginClass<BasicResource>("cBasicResource")
        .addFunction("GetName", &BasicResource::GetCharName)
        //.addFunction("GetDataReader", &ThisClass::GetDataReader)
	.endClass()
	;
}

XMLFile BasicResource::OpenMetaData() const {
	XMLFile xml;
	if (!GetFileSystem()->OpenResourceXML(xml, GetName(), DataPath::Fonts)) {
		AddLogf(Error, "Unable to open master resource xml for resource '%s' of class '%s'", GetName().c_str(), typeid(*this).name());
		return nullptr;
	}
	return xml;
}

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

DataClass::DataClass(): 
		m_Flags(0) {
}

DataClass::DataClass(const string& Name): BasicResource(Name), m_Flags(0) { 
}

DataClass::~DataClass() {
	AddLogf(Debug, "Destroying resource '%s'", GetName().c_str());
}

//---------------------------------------------------------------------------------------------------

bool DataClass::Initialize() {
	if (IsReady())
		return true;

	if (!DoInitialize()) {
		AddLogf(Error, "Unable to initialize resource '%s' of class '%s'", GetName().c_str(), typeid(*this).name());
		return false;
	}

	SetReady(true);
	AddLogf(Debug, "Initialized resource '%s' of class '%s'", GetName().c_str(), typeid(*this).name());
	return true;
}

bool DataClass::Finalize() {
	if (!IsReady())
		return true;

	SetReady(false);

	if (!DoFinalize()) {
		AddLogf(Error, "Unable to initialize resource '%s' of class '%s'", GetName().c_str(), typeid(*this).name());
		return false;
	}
	AddLogf(Debug, "Finalized resource resource '%s' of class '%s'", GetName().c_str(), typeid(*this).name());
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
