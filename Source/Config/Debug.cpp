#include <pch.h>
#ifdef DEBUG

#include "Core/Interfaces.h"

#include <Core/Scripts/LuaApi.h>
#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Component/EventInfo.h>
#include <Foundation/Component/ComponentInfo.h>

namespace Config {
namespace Debug {

bool EnableFlags::Physics = true;
bool EnableFlags::PhysicsDebugDraw = false;

bool EnableFlags::ShowTitleBarDebugInfo = true;

//---------------------------------------------------------------------------------------

void DebugSleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ScriptDebug(MoonGlare::Scripts::ApiInitializer &root){
	root
	.addFunction("Sleep", &DebugSleep)
	.beginNamespace("Flags")
		.beginNamespace("Physics")
			.addVariable("Enabled", &EnableFlags::Physics)
			.addVariable("DebugDraw", &EnableFlags::PhysicsDebugDraw)
		.endNamespace()
		.addVariable("ShowTitleBarDebugInfo", &EnableFlags::ShowTitleBarDebugInfo)
	.endNamespace();
	;
}

RegisterDebugApi(ScriptDebug, &ScriptDebug, "Debug");

//---------------------------------------------------------------------------------------
        		
} //namespace Debug
} //namespace Config

#endif
