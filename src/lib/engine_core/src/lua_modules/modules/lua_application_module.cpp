#include "lua_application_module.hpp"
#include "lua_context/lua_context_build_config.hpp"
#include <sol/sol.hpp>

namespace MoonGlare::LuaModules {

/*@ [LuaModules/LuaApplicationModule] Time module
    This module allows to control engine as application.
    `local time = require "moonglare.application"`
@*/

/*@ [LuaApplicationModule/LuaApplicationModuleMethods] Methods
@*/

/*@ [LuaApplicationModule/LuaApplicationModuleProperties] Fields
@*/

LuaApplicationModule::LuaApplicationModule(iEngineRunner *_engine_runner)
    : iDynamicScriptModule("LuaApplicationModule"), iRequireRequest("moonglare.application"),
      engine_runner(_engine_runner) {
}

LuaApplicationModule::~LuaApplicationModule() {
}

void LuaApplicationModule::InitContext(lua_State *lua) {
    sol::state_view sol_view(lua);
    auto ns = sol_view[Lua::kInternalLuaNamespaceName].get_or_create<sol::table>();

    ns.new_usertype<iEngineRunner>( //
        "iEngineRunner", sol::no_constructor,

        /*@ [LuaApplicationModuleProperties/_] 'Application.wants_soft_restart
            set to true if soft restart was requested @*/
        "wants_soft_restart", sol::property(&iEngineRunner::WantsSoftRestart),

        /*@ [LuaApplicationModuleMethods/_] `Application:set_soft_restart()`
            Set flag to perform soft restart when shutdown is requested @*/
        "set_soft_restart", &iEngineRunner::SetSoftRestart,

        /*@ [LuaApplicationModuleMethods/_] `Application:stop()`
            Perform gracefull engine shutdown or soft restart @*/
        "stop", &iEngineRunner::Stop,

        /*@ [LuaApplicationModuleProperties/_] `Application.compilation_date`
            Date of engine compilation @*/
        "compilation_date", sol::property(&iEngineRunner::GetCompilationDate),

        /*@ [LuaApplicationModuleProperties/_] 'Application.application_name
            Application name @*/
        "application_name", sol::property(&iEngineRunner::GetApplicationName),

        /*@ [LuaApplicationModuleProperties/_] 'Application.version_string
            Engine version @*/
        "version_string", sol::property(&iEngineRunner::GetVersionString));
}

bool LuaApplicationModule::OnRequire(lua_State *lua, std::string_view name) {
    sol::stack::push(lua, engine_runner);
    return true;
}

} // namespace MoonGlare::LuaModules
