#ifndef NCORE_H_
#define NCORE_H_

#include "Events.h"
#include "TimeEvents.h"

#include "DataManager.h"
#include "Scripts/nScripts.h"
#include "Scene/nScene.h"

#include "Component/nfComponent.h"

//core shortcut getters
namespace MoonGlare {
namespace Core {
	inline Scene::ScenesManager* GetScenesManager() { return Scene::ScenesManager::Instance(); }
} //namespace Core 
} //namespace MoonGlare 

#endif // NCORE_H_
