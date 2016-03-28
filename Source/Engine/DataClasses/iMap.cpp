/*
 * ciMap.cpp
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace DataClasses {
namespace Maps {

GABI_IMPLEMENT_ABSTRACT_CLASS(iMap)
IMPLEMENT_SCRIPT_EVENT_VECTOR(MapScriptEvents);
RegisterApiDerivedClass(iMap, &iMap::RegisterScriptApi);

iMap::iMap(const string &Name) :
		BaseClass(),
		m_OwnerScene(nullptr),
		m_PathRegister(std::make_unique<Paths::PathRegister>(this)) {
	SetName(Name);
}

iMap::~iMap() {
}

void iMap::SetOwnerScene(::Core::Scene::GameScene* Scene) {
	m_OwnerScene = Scene;
}

//----------------------------------------------------------------

void iMap::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("ciMap")
		//.addFunction("GetTexture", &DataModule::GetTexture_char)
		//.addFunction("GetPatternObject", &DataModule::GetPatternObject_char)
	.endClass();
}

//-------------------------------------------------------------------------------------------------

bool iMap::Initialize() {
	if (IsReady()) return true;
	if (!DoInitialize()) {
		AddLog(Error, "Unable to initialize map " << GetName());
		return false;
	}
	if (InvokeOnInitialize() != 0)
		return false;
	SetReady(true);
	return true;
}

bool iMap::Finalize() {
	if (!IsReady()) return true;
	if (!DoFinalize()) {
		AddLog(Error, "Unable to finalize map " << GetName());
		return false;
	}
	if(InvokeOnFinalize() != 0)
		return false;
	SetReady(false);
	return true;
}

bool iMap::DoInitialize() {
	m_PathRegister->Initialize();
	return true;
}

bool iMap::DoFinalize() {
	m_PathRegister->Finalize();
	return true;
}

//----------------------------------------------------------------

int iMap::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int iMap::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }
int iMap::InvokeAfterLoad() { SCRIPT_INVOKE(AfterLoad, GetOwnerScene()); }

//----------------------------------------------------------------

xml_node iMap::GetPrimaryXML() {
	if (!m_MapMeta)
		GetFileSystem()->OpenResourceXML(m_MapMeta, GetName(), DataPath::Maps);
	return m_MapMeta->document_element();
}

bool iMap::LoadMapObjects(::Core::Objects::ObjectRegister& where) {
	auto node = GetPrimaryXML();
	if (!node) return false;
	auto MapData = node.child("Objects");
	return where.LoadObjects(MapData, GetOwnerScene());
}

//----------------------------------------------------------------

bool iMap::LoadEnvironment(const xml_node SrcNode) {
	auto reader = GetDataReader();
	return m_Environment.LoadMeta(SrcNode.child("Environment"), reader);
}

bool iMap::LoadEvents(const xml_node RootNode) {
	m_ScriptHandlers->LoadFromXML(RootNode);
	return true;
}

} // namespace Maps
} // namespace DataClasses
