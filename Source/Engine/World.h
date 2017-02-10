/*
  * Generated by cppsrc.sh
  * On 2016-05-03 21:31:13,10
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef World_H
#define World_H

#include "Core/EntityManager.h"
#include "Core/HandleTable.h"
#include "Core/ResourceTable.h"

namespace MoonGlare {

class World {
public:
 	World();
 	~World();

	bool Initialize(Core::Scripts::ScriptEngine *se);
	bool Finalize();
	bool PostSystemInit();
	bool PreSystemStart();
	bool PreSystemShutdown();

	bool Step(const Core::MoveConfig &config);

	Core::EntityManager* GetEntityManager() { return &m_EntityManager; }
	Core::HandleTable* GetHandleTable() { return &m_HandleTable; }
	Core::ResourceTable* GetResourceTable() { return &m_ResourceTable; }
	Core::InputProcessor* GetInputProcessor() { return m_InputProcessor.get(); }

	Core::Scripts::ScriptEngine *GetScriptEngine() { return m_ScriptEngine; };
	Core::Scene::ScenesManager *GetScenesManager() { return m_ScenesManager.get(); };

	Core::Hooks* GetHooks() { return m_Hooks.get(); }

	Core::Console* GetConsole();
    Core::Engine* GetEngine();
private:
	std::unique_ptr<Core::InputProcessor> m_InputProcessor;
	std::unique_ptr<Core::Scene::ScenesManager> m_ScenesManager;
	std::unique_ptr<Core::Hooks> m_Hooks;

	Core::Scripts::ScriptEngine *m_ScriptEngine;

	Core::HandleTable m_HandleTable;
	Core::ResourceTable m_ResourceTable;
	Core::EntityManager m_EntityManager;
};

} //namespace MoonGlare

#endif
