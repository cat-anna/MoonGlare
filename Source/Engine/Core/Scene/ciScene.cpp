/*
 * ciScene.cpp
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/GUI/nGUI.h>
#include <Engine/iSoundEngine.h>

namespace MoonGlare {
namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(ciScene)
IMPLEMENT_SCRIPT_EVENT_VECTOR(SceneScriptEvents);
RegisterApiDerivedClass(ciScene, &ciScene::RegisterScriptApi);
inline ciScene* CurrentInstance() { return GetEngine()->GetCurrentScene(); }
RegisterApiDynamicInstance(ciScene, &CurrentInstance, "Scene");

ciScene::ciScene() :
		BaseClass(),
		m_SceneState(SceneState::Waiting),
		m_ScriptHandlers(),
		m_Flags(0),
		m_TimeEvents(),
		m_Camera() {
	m_EventProxy.set(new EventProxy<ThisClass, 
					 &ThisClass::InvokeOnTimer,
					 &ThisClass::InternalEventNotification>(this));
}

ciScene::~ciScene() {
	//TODO: delete ScritpHanldlers;
}

void ciScene::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("iScene")
		.addFunction("SetTimer", &ThisClass::SetTimer)
		.addFunction("KillTimer", &ThisClass::KillTimer)

		.addFunction("InvokeOnTimer", &ThisClass::InvokeOnTimer)
		.addFunction("SetEventFunction", &ThisClass::SetEventFunction)

		.addFunction("FinishScene", &ThisClass::FinishScene)
		.addFunction("SetFinishedState", &ThisClass::SetFinishedState)
		
		.addFunction("GetGUI", Utils::Template::SmartPointerTweeks<ThisClass, GUI::GUIEnginePtr>::Get<&ThisClass::m_GUI>())
	.endClass();
}

//----------------------------------------------------------------

void ciScene::BeginScene() {
	THROW_ASSERT(IsInitialized() && !IsReady(), 0);

	if(Sound::iSoundEngine::InstanceExists())
		GetSoundEngine()->SetPlayList(GetSettings().PlayList);

	if (m_GUI)
		m_GUI->Activate();

	SetReady(true);
	InvokeOnBeginScene();
}

void ciScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);

	if (m_GUI)
		m_GUI->Deactivate();

	InvokeOnEndScene();
	SetReady(false);
}

bool ciScene::Initialize() {
	if (IsInitialized()) return true;
	AddLog(Debug, "Initializing scene: " << GetName());
	if (!DoInitialize()) {
		AddLog(Error, "Unable to initialize scene " << GetName());
		return false;
	}
	if (InvokeOnInitialize() != 0)
		return false;
	SetInitialized(true);
	return true;
}

bool ciScene::Finalize() {
	if (!IsInitialized()) return true;
	if (IsReady()) EndScene();
	AddLog(Debug, "Finalizing scene: " << GetName());
	if (!DoFinalize()) {
		AddLog(Error, "Unable to finalize scene " << GetName());
		return false;
	}
	if(InvokeOnFinalize() != 0)
		return false;
	SetInitialized(false);
	return true;
}

bool ciScene::DoInitialize() {
	if (!m_ComponentManager.Initialize(this)) {
		AddLogf(Error, "Failed to initialize component manager");
		return false;
	}

	return true;
}

bool ciScene::DoFinalize() {
	if (m_GUI) m_GUI->Finalize();
	m_GUI.reset();

	if (!m_ComponentManager.Finalize()) {
		AddLogf(Error, "Failed to finalize component manager");
		return false;
	}

	return true;
}

//----------------------------------------------------------------

void ciScene::DoMove(const MoveConfig &conf) {
	conf.Scene = this;
	m_TimeEvents.CheckEvents(conf);
	if (m_GUI)
		m_GUI->Process(conf);
	if (m_Camera)
		m_Camera->Update(conf);

	m_ComponentManager.Step(conf);
}

Graphic::Light::LightConfiguration* ciScene::GetLightConfig() {
	//nothing here
	return 0;
}

//----------------------------------------------------------------

int ciScene::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int ciScene::InvokeOnEscape() { SCRIPT_INVOKE(OnEscape); }
int ciScene::InvokeOnBeginScene() { SCRIPT_INVOKE(OnBeginScene); }
int ciScene::InvokeOnEndScene() { SCRIPT_INVOKE(OnEndScene); }
int ciScene::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int ciScene::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }

//----------------------------------------------------------------

bool ciScene::SetMetaData(FileSystem::XMLFile &file) {
	m_MetaData.swap(file);
	if (!m_MetaData) return false;
	return LoadMeta(GetRootNode());
}

bool ciScene::LoadMeta(const xml_node Node) {
	SetName(Node.attribute(xmlAttr_Name).as_string("??"));
	m_ScriptHandlers->LoadFromXML(Node);
	m_Settings.LoadMeta(Node.child("Settings"));
	return m_ComponentManager.LoadComponents(Node.child("Components"));
}

const xml_node ciScene::GetRootNode() const { 
	if (!m_MetaData)
		return xml_node();
	return m_MetaData->document_element();
}

//----------------------------------------------------------------

void ciScene::FinishScene() {
	SetSceneState(SceneState::Finished);
	GetEngine()->ChangeScene();
}

void ciScene::SetFinishedState() {
	SetSceneState(SceneState::Finished);
}

void ciScene::SetSceneState(SceneState state) {
	m_SceneState = state;
	//GetEngine()->HandleSceneStateChange();
}

int ciScene::InternalEventNotification(Events::InternalEvents event, int Param) {
	//No events are handled here
	return 0;
}

//----------------------------------------------------------------
//----------------------------------------------------------------

SceneSettings::SceneSettings(): PlayList("") {

}

bool SceneSettings::LoadMeta(const xml_node node) {
	PlayList = node.child("PlayList").text().as_string("");
	return true;
}

} // namespace Scene
} // namespace Core
} // namespace MoonGlare
