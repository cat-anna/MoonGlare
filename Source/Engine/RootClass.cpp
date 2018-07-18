#include <pch.h>
#include <nfMoonGlare.h>

#include <Core/Scripts/LuaApi.h>

SPACERTTI_IMPLEMENT_CLASS(cRootClass);
RegisterApiBaseClass(cRootClass, &cRootClass::RegisterScriptApi);

void cRootClass::RegisterScriptApi(ApiInitializer &api) {
	api.beginClass<ThisClass>("cRootClass")
//#ifndef _DISABLE_SCRIPT_ENGINE_
		//.addCFunction("ExactClass", &ThisClass::PushExactClass)
//#endif
	.endClass();
}
