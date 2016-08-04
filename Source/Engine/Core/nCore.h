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

//core shortcut getters
namespace MoonGlare {
namespace Core {
	inline Engine* GetEngine() { return Engine::Instance(); }
	inline ::Core::Scripts::cScriptEngine* GetScriptEngine() { return ::Core::Scripts::cScriptEngine::Instance(); }
	//inline cGameEngine* GetGameEngine() { return cGameEngine::Instance(); }

	inline Scene::ScenesManager* GetScenesManager() { return Scene::ScenesManager::Instance(); }

	inline ::Core::Input* GetInput() { return ::Core::Input::Instance(); }
}
} //namespace MoonGlare 

#endif // NCORE_H_
