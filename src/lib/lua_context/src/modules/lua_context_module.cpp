#include "lua_context/modules/lua_context_module.hpp"
#include "lua_context/lua_context_build_config.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
#include <sol/sol.hpp>

namespace MoonGlare::Lua {

LuaContextModule::LuaContextModule(iScriptContext *_lua_context)
    : iDynamicScriptModule("LuaContextModule"), iRequireRequest("moonglare.lua_context"), lua_context(_lua_context) {
}

LuaContextModule::~LuaContextModule() {
}

/*@ [LuaModules/LuaContextModule] LuaContext module
    This module allows control lua script context
    `local lua_context = require "moonglare.lua_context"`
@*/

/*@ [LuaContextModule/LuaContextModuleMethods] Methods
@*/

/*@ [LuaContextModule/LuaContextModuleProperties] Fields
@*/

void LuaContextModule::InitContext(lua_State *lua) {
    sol::state_view sol_view(lua);
    auto ns = sol_view[kInternalLuaNamespaceName].get_or_create<sol::table>();
    ns.new_usertype<iScriptContext>( //
        "LuaContextModule", sol::no_constructor,

        /*@ [LuaContextModuleMethods/_] `LuaContext:collect_garbage()`
            Perform full garbage collection cycle @*/
        "collect_garbage", &iScriptContext::CollectGarbage,

        /*@ [LuaContextModuleProperties/_] `LuaContext.memory_usage`
            Get current amount of memory used by script context @*/
        "memory_usage", sol::property(&iScriptContext::GetMemoryUsage),

        /*@ [LuaContextModuleProperties/_] `LuaContext.gc_step`
            Set/get gc step size @*/
        "gc_step", sol::property(&iScriptContext::GetGcStepSize, &iScriptContext::SetGcStepSize),

        /*@ [LuaContextModuleProperties/_] `LuaContext.lua_version`
            Lua version @*/
        "lua_version", sol::var(lua_version(lua))
        //
    );
}

bool LuaContextModule::OnRequire(lua_State *lua, std::string_view name) {
    sol::stack::push(lua, lua_context);
    return true;
}

} // namespace MoonGlare::Lua
