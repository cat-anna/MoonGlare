#if 0
#include <Foundation/Scripts/LuaStackOverflowAssert.h>
#include <interface_map.h>

#include "../ApiInit.h"
#include "../iDynamicModule.h"
#include "../iLuaRequire.h"

#include "LuaAsyncLoader.h"

namespace MoonGlare::Scripts::Modules {

LuaAsyncLoaderModule::LuaAsyncLoaderModule(lua_State *lua, InterfaceMap *world)
    : interfaceMap(*world) {
    world->GetInterface<iRequireModule>()->RegisterRequire("AsyncLoader", this);
}

LuaAsyncLoaderModule::~LuaAsyncLoaderModule() {}

/*@ [RequireModules/LuaAsyncLoaderModule] AsyncLoader module
    This module allows to access internal async loader queue to get its status.
@*/
ApiInitializer LuaAsyncLoaderModule::RegisterScriptApi(ApiInitializer api) {
    return api
        .beginClass<LuaAsyncLoaderModule>("LuaAsyncLoaderModule")
        /*@ [LuaAsyncLoaderModule/_] `AsyncLoader:GetStatus()`
            Get async loader status. Returned object contain fields:
            * status.pendingJobs - count of queued jobs
            * status.jobCount - last known job count. This value wiil be reset
        to 0 after all pending jobs are done
        @*/
        .addFunction("GetStatus", &LuaAsyncLoaderModule::GetStatus)
        .endClass()
        .beginClass<Resources::iAsyncLoader::JobStatus>("AsyncLoaderJobStatus")
        .addData("pendingJobs",
                 &Resources::iAsyncLoader::JobStatus::pendingJobs)
        .addData("jobCount", &Resources::iAsyncLoader::JobStatus::localJobCount)
        .endClass();
}

bool LuaAsyncLoaderModule::OnRequire(lua_State *lua, std::string_view name) {
    luabridge::push<LuaAsyncLoaderModule *>(lua, this);
    return true;
}

//-------------------------------------------------------------------------------------------------

Resources::iAsyncLoader::JobStatus LuaAsyncLoaderModule::GetStatus() const {
    return interfaceMap.GetInterface<Resources::iAsyncLoader>()->GetJobStatus();
}

} // namespace MoonGlare::Scripts::Modules
#endif