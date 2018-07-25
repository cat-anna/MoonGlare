/*
 * ciScene.cpp
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/Engine.h>
#include <Core/Scripts/LuaApi.h>


#include <Core/EntityBuilder.h>

#include "Scene.Events.h"

namespace MoonGlare {
namespace Core {
namespace Scene {

RegisterApiNonClass(ciScene, &ciScene::RegisterScriptApi);

void ciScene::RegisterScriptApi(ApiInitializer &api) {
	api
    .beginNamespace("api")
	    .beginClass<ciScene>("iScene")
		    .addFunction("SpawnChild", &ciScene::SpawnChildRaw)
	    .endClass();
}

bool ciScene::SpawnChildRaw(const char * URI, const char * Name) {
	Entity e;
	AddLogf(Debug, "Spawning child: '%s' from '%s'", Name ? Name : "?", URI ? URI : "?");
	return EntityBuilder(&m_SubsystemManager).Build(m_SubsystemManager.GetRootEntity(), URI, e, Name) ;
}

bool ciScene::SpawnChild(const std::string & URI, std::string Name, Entity & out) {
	AddLogf(Debug, "Spawning child: '%s' from '%s'", Name.c_str(), URI.c_str());
	return EntityBuilder(&m_SubsystemManager).Build(m_SubsystemManager.GetRootEntity(), URI.c_str(), out, std::move(Name)) ;
}
ciScene::ciScene() { }
ciScene::~ciScene() { }

//----------------------------------------------------------------

void ciScene::SendState(SceneState state) {
    m_SubsystemManager.GetEventDispatcher().Send<SceneStateChangeEvent>({ state, this });
}

void ciScene::BeginScene() {
    SendState(SceneState::Started);
}

void ciScene::EndScene() {
    SendState(SceneState::Paused);
}

bool ciScene::Initialize(pugi::xml_node Node, std::string Name, Entity OwnerEntity, SceneDescriptor *Descriptor) {
    ASSERT(Descriptor);
    
    AddLog(Debug, "Initializing scene: " << Name);

    m_Descriptor = Descriptor;

    Entity root;
    auto em = GetEngine()->GetWorld()->GetEntityManager();
    if (!em->Allocate(OwnerEntity, root, std::move(Name))) {
        AddLogf(Error, "Failed to allocate scene entity!");
        return false;
    }

    if (!m_SubsystemManager.LoadComponents(Node.child("Components"))) {
        AddLogf(Error, "Failed to load components");
        return false;
    }

    if (!m_SubsystemManager.Initialize(this, root)) {
        AddLogf(Error, "Failed to initialize component manager");
        return false;
    }

    SendState(SceneState::Created);                  

    EntityBuilder(&m_SubsystemManager).Build(root, Node.child("Entities"));

    return true;
}

bool ciScene::Finalize() {
    //SendState(SceneState::);

    if (!m_SubsystemManager.Finalize()) {
        AddLogf(Error, "Failed to finalize component manager");
        return false;
    }

    return true;
}

//----------------------------------------------------------------

void ciScene::DoMove(const MoveConfig &conf) {
    m_SubsystemManager.Step(conf);
}

//----------------------------------------------------------------

} // namespace Scene
} // namespace Core
} // namespace MoonGlare
