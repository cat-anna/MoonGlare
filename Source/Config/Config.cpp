#include <pch.h>
#include <nfMoonGlare.h>

#include <Core/Scripts/LuaApi.h>

namespace Config {

static const char* GetConfigName() { return CONFIGURATION_NAME; }

void ConfigScriptApi(MoonGlare::Scripts::ApiInitializer &root) {
	root
		.addProperty("Debug", &Utils::Template::StaticReturn<bool, DEBUG_TRUE>, (void(*)(bool))0)
		.addProperty("Name", &GetConfigName, (void(*)(const char*))0)
		;
}

RegisterApiNonClass(ConfigScriptApi, &ConfigScriptApi, "Config");

} //namespace Config