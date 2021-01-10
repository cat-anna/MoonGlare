#include "lua_require_handler.hpp"
#include "lua_context_build_config.hpp"
#include "lua_exec_string.hpp"
#include <array>
#include <embedded/lua_require_handler_debug.lua.h>
#include <fmt/format.h>
#include <lua.hpp>
#include <lua_context/lua_panic.hpp>
#include <orbit_logger.h>

namespace MoonGlare::Lua {

//-------------------------------------------------------------------------------------------------

/*@ [StaticModules/RequireModule] Require module
    Module is accessible directly via global *require* variable.
    Extensions are available only in debug mode.
@*/

LuaRequireModule::LuaRequireModule(std::shared_ptr<iReadOnlyFileSystem> filesystem)
    : filesystem(std::move(filesystem)) {
}

LuaRequireModule::~LuaRequireModule() {
}

void LuaRequireModule::InitContext(lua_State *lua) {
    AddLogf(Debug, "Initializing Require module");

    lua_newtable(lua); // table for cache           // cache

    if constexpr (kEnableDebugScriptApi) {
        lua_pushvalue(lua, -1);              // cache cache
        lua_setglobal(lua, "require_cache"); // cache
    }

    lua_pushlightuserdata(lua, this);     // cache this
    lua_pushvalue(lua, -2);               // cache this cache
    lua_settable(lua, LUA_REGISTRYINDEX); // cache

    lua_pushlightuserdata(lua, this);       // cache this
    lua_pushvalue(lua, -2);                 // cache this cache
    lua_pushcclosure(lua, &lua_require, 2); // cache closure
    lua_setglobal(lua, "require");          // cache

    lua_pushlightuserdata(lua, this);      // cache this
    lua_pushvalue(lua, -2);                // cache this cache
    lua_pushcclosure(lua, &lua_dofile, 2); // cache closure
    lua_setglobal(lua, "dofile");          // cache

    lua_pop(lua, 1);

    if constexpr (kEnableDebugScriptApi) {
        if (!ExecuteString(lua, lua_require_handler_debug_lua, lua_require_handler_debug_lua_size,
                           "lua_require_handler_debug.lua")) {
            throw std::runtime_error("lua_require_handler_debug.lua execute code failed!");
        }
    }
}

//-------------------------------------------------------------------------------------------------

void LuaRequireModule::RegisterRequire(std::string name, iRequireRequest *iface) {
    if (!iface) {
        AddLogf(Debug, "Unregistered lua require: %s", name.c_str());
        scriptRequireMap.erase(name);
        return;
    }
    AddLogf(Debug, "Registered lua require: %s", name.c_str());
    scriptRequireMap[name] = iface;
}

bool LuaRequireModule::Query(lua_State *lua, std::string_view name) {
    lua_pushlightuserdata(lua, this);
    lua_gettable(lua, LUA_REGISTRYINDEX);
    int tableidx = lua_gettop(lua);

    bool succ = ProcessRequire(lua, name, tableidx);
    if (succ) {
        lua_insert(lua, -2);
    }

    lua_pop(lua, 1);
    return succ;
}

bool LuaRequireModule::ProcessRequire(lua_State *lua, std::string_view name, int cachetableloc) {

    lua_getfield(lua, cachetableloc, name.data());
    if (!lua_isnil(lua, -1)) {
        AddLog(Performance, fmt::format("Got require '{}' from cache", name.data()));
        return 1;
    } else {
        lua_pop(lua, 1);
    }

    static constexpr std::array<decltype(&LuaRequireModule::HandleFileRequest), 2> funcs = {
        &LuaRequireModule::HandleFileRequest,
        &LuaRequireModule::HandleModuleRequest,
    };
    const char *t;
    for (auto item : funcs) {
        auto mode = (this->*item)(lua, name);
        t = lua_typename(lua, lua_type(lua, -1));

        switch (mode) {
        case ResultStoreMode::NoResult:
            continue;
        case ResultStoreMode::Store:
            lua_pushvalue(lua, -1);
            t = lua_typename(lua, lua_type(lua, -1));
            lua_setfield(lua, cachetableloc, name.data());
            t = lua_typename(lua, lua_type(lua, -1));
            return true;
        case ResultStoreMode::DontStore:
            return true;
        default:
            LogInvalidEnum(mode);
            break;
        }
    }

    AddLog(Warning, fmt::format("Cannot find require '{}'", name.data()));
    return false;
}

LuaRequireModule::ResultStoreMode LuaRequireModule::TryLoadFileScript(lua_State *lua, const std::string &file_path) {
    std::string file_data;
    if (!filesystem->ReadFileByPath(file_path, file_data)) {
        AddLogf(Warning, "Cannot open file %s", file_path.c_str());
        return ResultStoreMode::NoResult;
    }

    if (!MoonGlare::Lua::ExecuteString(lua, file_data.c_str(), file_data.size(), file_path.c_str(), 1)) {
        AddLogf(Error, "Script execution failed: %s", file_path.c_str());
        return ResultStoreMode::NoResult;
    }

    return ResultStoreMode::Store;
}

LuaRequireModule::ResultStoreMode LuaRequireModule::HandleFileRequest(lua_State *lua, std::string_view name) {
    // TODO: loading compiled scripts
    return TryLoadFileScript(lua, std::string(name) + ".lua");
}

LuaRequireModule::ResultStoreMode LuaRequireModule::HandleModuleRequest(lua_State *lua, std::string_view name) {
    auto it = scriptRequireMap.find(name.data());
    if (it != scriptRequireMap.end()) {
        if (it->second->OnRequire(lua, name))
            return ResultStoreMode::DontStore;
    }
    return ResultStoreMode::NoResult;
}

//-------------------------------------------------------------------------------------------------

int LuaRequireModule::lua_require(lua_State *lua) {
    void *voidThis = lua_touserdata(lua, lua_upvalueindex(1));
    LuaRequireModule *This = reinterpret_cast<LuaRequireModule *>(voidThis);

    std::string_view name = luaL_checkstring(lua, -1);
    if (This->ProcessRequire(lua, name, lua_upvalueindex(2))) {
        // auto t = lua_typename(lua, lua_type(lua, -1));
        return 1;
    }

    throw LuaPanic(fmt::format("Cannot find require '{}'", lua_tostring(lua, -1)));
}

int LuaRequireModule::lua_dofile(lua_State *lua) {
    void *voidThis = lua_touserdata(lua, lua_upvalueindex(1));
    LuaRequireModule *This = reinterpret_cast<LuaRequireModule *>(voidThis);

    std::string_view name = luaL_checkstring(lua, -1);

    if (name[0] != '/') {
        throw LuaPanic(fmt::format("Invalid file path '{}'", name.data()));
    }

    std::string uri = std::string(name) + ".lua";
    switch (This->TryLoadFileScript(lua, uri)) {
    case ResultStoreMode::Store:
    case ResultStoreMode::DontStore:
        return 1;
    default:
    case ResultStoreMode::NoResult:
        throw LuaPanic(fmt::format("Cannot load file '{}'", name.data()));
    }
}

} // namespace MoonGlare::Lua