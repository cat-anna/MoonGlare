/*
 * ciScene.cpp
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iSoundEngine.h>
#include <Engine/Core/Engine.h>

#include <Core/EntityBuilder.h>

namespace MoonGlare {
namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_STATIC_CLASS(ciScene)
RegisterApiDerivedClass(ciScene, &ciScene::RegisterScriptApi);
inline ciScene* CurrentInstance() { return GetEngine()->GetCurrentScene(); }
RegisterApiDynamicInstance(ciScene, &CurrentInstance, "Scene");
SceneClassRegister::Register<ciScene> SceneReg("Scene");

ciScene::ciScene() :
		BaseClass(),
		m_SceneState(SceneState::Waiting),
		m_Flags(0),
		m_TimeEvents() {
}

ciScene::~ciScene() {
}

void ciScene::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("iScene")
		.addFunction("FinishScene", &ThisClass::FinishScene)
		.addFunction("SetFinishedState", &ThisClass::SetFinishedState)
	.endClass();
}

//----------------------------------------------------------------

void ciScene::BeginScene() {
	THROW_ASSERT(IsInitialized() && !IsReady(), 0);

	Graphic::GetRenderDevice()->BindEnvironment(&m_Environment);
	m_Environment.Initialize();

	SetReady(true);
}

void ciScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);

	Graphic::GetRenderDevice()->BindEnvironment(nullptr);
	m_Environment.Finalize();

	SetReady(false);
}

bool ciScene::Initialize() {
	if (IsInitialized()) return true;
	AddLog(Debug, "Initializing scene: " << GetName());
	if (!DoInitialize()) {
		AddLog(Error, "Unable to initialize scene " << GetName());
		return false;
	}
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
	SetInitialized(false);
	return true;
}

bool ciScene::DoInitialize() {
	if (!m_ComponentManager.Initialize(this)) {
		AddLogf(Error, "Failed to initialize component manager");
		return false;
	}

	auto node = GetRootNode().child("Entities");
	EntityBuilder eb(&m_ComponentManager);
	auto root = GetEngine()->GetWorld()->GetEntityManager()->GetRootEntity();
	eb.ProcessXML(root, node);

	return true;
}

bool ciScene::DoFinalize() {
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
	m_ComponentManager.Step(conf);
}

//----------------------------------------------------------------

bool ciScene::SetMetaData(FileSystem::XMLFile &file) {
	m_MetaData.swap(file);
	if (!m_MetaData) return false;
	return LoadMeta(GetRootNode());
}

bool ciScene::LoadMeta(const xml_node Node) {
	SetName(Node.attribute(xmlAttr_Name).as_string("??"));
	m_Environment.LoadMeta(Node.child("Environment"));
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

} // namespace Scene
} // namespace Core
} // namespace MoonGlare
