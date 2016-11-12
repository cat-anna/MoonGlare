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

#include "Scene.Events.h"

namespace MoonGlare {
namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_STATIC_CLASS(ciScene)
RegisterApiDerivedClass(ciScene, &ciScene::RegisterScriptApi);

ciScene::ciScene() :
		BaseClass(),
		m_Descriptor(nullptr),
		m_Flags(0) {
}

ciScene::~ciScene() {
}

void ciScene::RegisterScriptApi(ApiInitializer &api) {
	//api
	//.deriveClass<ThisClass, BaseClass>("iScene")
	//.endClass();
}

//----------------------------------------------------------------

void ciScene::BeginScene() {
	THROW_ASSERT(IsInitialized() && !IsReady(), 0);

	Graphic::GetRenderDevice()->BindEnvironment(&m_Environment);
	m_Environment.Initialize();

	SendState(SceneState::Started);

	SetReady(true);
}

void ciScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);

	SendState(SceneState::Paused);

	Graphic::GetRenderDevice()->BindEnvironment(nullptr);
	m_Environment.Finalize();

	SetReady(false);
}

bool ciScene::Initialize(pugi::xml_node Node, std::string Name, Entity OwnerEntity, SceneDescriptor *Descriptor) {
	if (IsInitialized()) return true;
	
	ASSERT(Descriptor);
	
	AddLog(Debug, "Initializing scene: " << Name);

	m_Descriptor = Descriptor;

	auto em = GetEngine()->GetWorld()->GetEntityManager();
	if (!em->Allocate(OwnerEntity, m_Entity, std::move(Name))) {
		AddLogf(Error, "Failed to allocate scene entity!");
		return false;
	}

	if (!m_ComponentManager.LoadComponents(Node.child("Components"))) {
		AddLogf(Error, "Failed to load components");
		return false;
	}

	if (!m_ComponentManager.Initialize(this)) {
		AddLogf(Error, "Failed to initialize component manager");
		return false;
	}

	//SetName(Node.attribute(xmlAttr_Name).as_string("??"));
	m_Environment.LoadMeta(Node.child("Environment"));

	SendState(SceneState::Created);

	EntityBuilder(&m_ComponentManager).ProcessXML(GetSceneEntity(), Node.child("Entities"));

	SetInitialized(true);
	return true;
}

bool ciScene::Finalize() {
	if (!IsInitialized()) return true;
	if (IsReady()) EndScene();

	//SendState(SceneState::);

	if (!m_ComponentManager.Finalize()) {
		AddLogf(Error, "Failed to finalize component manager");
		return false;
	}

	SetInitialized(false);
	return true;
}

//----------------------------------------------------------------

void ciScene::DoMove(const MoveConfig &conf) {
	m_ComponentManager.Step(conf);
}

//----------------------------------------------------------------

} // namespace Scene
} // namespace Core
} // namespace MoonGlare
