#ifndef NCORE_H_
#define NCORE_H_

#include "Input.h"
#include "Events.h"
#include "TimeEvents.h"

#include "DataManager.h"
#include "Scripts/nScripts.h"
#include "Objects/nObjects.h"
#include "Scene/nScene.h"
#include "Camera/nCamera.h"

#include "Component/nfComponent.h"

#include "Engine.h"
#include "Interfaces.h"

//core shortcut getters
namespace Core {
	inline Engine* GetEngine() { return ::Core::Engine::Instance(); }
	inline cScriptEngine* GetScriptEngine() { return Scripts::cScriptEngine::Instance(); }
	//inline cGameEngine* GetGameEngine() { return cGameEngine::Instance(); }

	inline Scene::ScenesManager* GetScenesManager() { return Scene::ScenesManager::Instance(); }

	inline Input* GetInput() { return Input::Instance(); }
}

#endif // NCORE_H_
