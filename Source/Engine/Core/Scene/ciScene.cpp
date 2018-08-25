/*
 * ciScene.cpp
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/Core/Engine.h>
#include <Core/Scripts/LuaApi.h>
#include <Core/EntityBuilder.h>
                                   
#include "ciScene.h"
#include "Scene.Events.h"

#include <Foundation/Component/EntityManager.h>

namespace MoonGlare {
namespace Core {
namespace Scene {

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

bool ciScene::Initialize(pugi::xml_node Node, std::string Name, Entity OwnerEntity) {
    AddLog(Debug, "Initializing scene: " << Name);

    Entity root;
    auto &em = GetEngine()->GetWorld()->GetEntityManager();
    if (!em.Allocate(OwnerEntity, root, std::move(Name))) {
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

    em.RegisterEventSink(&m_SubsystemManager.GetEventDispatcher());

    SendState(SceneState::Created);                  

    EntityBuilder(&m_SubsystemManager).Build(root, Node.child("Entities"));

    return true;
}

bool ciScene::Finalize() {
    auto &em = GetEngine()->GetWorld()->GetEntityManager();
    em.UnregisterEventSink(&m_SubsystemManager.GetEventDispatcher());

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
