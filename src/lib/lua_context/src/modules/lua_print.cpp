
#include "lua_context/modules/lua_print.hpp"
#include "lua_exec_string.hpp"
#include <orbit_logger.h>
#include <sol/sol.hpp>

#include <embedded/lua_print.lua.h>

namespace MoonGlare::Lua {

template <int line_type>
static int lua_put(lua_State *L) {
    const char *msg = luaL_checkstring(L, -1);
    if (!msg || *msg == '\0') {
        return 0;
    }
    switch (line_type) {
    case 0:
        AddLog(Error, msg);
        break;
    case 1:
        AddLog(Warning, msg);
        break;
    case 3:
        AddLog(Debug, msg);
        break;
    case 2:
    default:
        AddLog(Info, msg);
        break;
    }
    return 0;
}

LuaPrintModule::LuaPrintModule() : iDynamicScriptModule("LuaPrintModule") {
}

/*@ [LuaModules/LuaPrintModule] Print&Logging module
Provide printing to engine console or to logs.
Partially accessible through global `log`
@*/

void LuaPrintModule::InitContext(lua_State *lua) {
    AddLogf(Debug, "Initializing Print module");

    /*@ [LuaPrintModule/_] Log global namespace
Provides access to primary log methods, does not do any text formatting. This function should not be used directly:

* `log.error(text)`
* `log.warning(text)`
* `log.info(text)`
* `log.debug(text)`
@*/
    sol::state_view sol_view(lua);
    auto log = sol_view["log"].get_or_create<sol::table>();
    log["error"] = &lua_put<0>;
    log["warning"] = &lua_put<1>;
    log["info"] = &lua_put<2>;
    log["debug"] = &lua_put<3>;

    if (!ExecuteString(lua, lua_print_lua, lua_print_lua_size, "lua_print.lua")) {
        throw std::runtime_error("lua_print.lua execute code failed!");
    }
}

} // namespace MoonGlare::Lua
