#include <pch.h>
#include <nfMoonGlare.h>
#include <iConsole.h>

#include "../ScriptEngine.h"
#include "StaticModules.h"

namespace MoonGlare::Core::Scripts::Modules {

template<iConsole::LineType lt>
static int Lua_put(lua_State *L, World *w) {
    const char *msg = luaL_checkstring(L, -1);
    if (!msg || *msg == '\0')
        return 0;
    switch (lt) {
    case MoonGlare::iConsole::LineType::Error:
        AddLog(Debug, msg);
        break;
    case MoonGlare::iConsole::LineType::Warning:
        AddLog(Warning, msg);
        break;
    case MoonGlare::iConsole::LineType::Hint:
        AddLog(Hint, msg);
        break;
    case MoonGlare::iConsole::LineType::Debug:
        AddLog(Debug, msg);
        break;
    default:
        break;
    }                   
    if (w) {
        auto c = w->GetConsole();
        if (c)
            c->AddLine(msg, lt);
    }
    return 0;
}

template<typename T, int(*F)(lua_State*, T* ptr)>
int ClosureWrap(lua_State* lua) {
    T *t = reinterpret_cast<T*>(lua_touserdata(lua, lua_upvalueindex(1)));
    return F(lua, t);
}

static constexpr char *InitPrint = R"===(

local function dofmt(...)
    local t = { }
    for i,v in ipairs({...}) do
        t[i] = tostring(v)
    end
    return table.concat(t, " ")
end

function print(...) Log.Console(dofmt(...)) end
function warning(...) Log.Warning(dofmt(...)) end
function hint(...) Log.Hint(dofmt(...)) end
function debug(...) Log.Debug(dofmt(...)) end

function printf(...) Log.Console(string.format(...)) end
function warningf(...) Log.Warning(string.format(...)) end
function hintf(...) Log.Hint(string.format(...)) end
function debugf(...) Log.Debug(string.format(...)) end

local lua_error = error
function error(msg, c) 
    Log.Error(msg) 
    lua_error(t, (c or 0) + 1)
end
                   
)===";

void StaticModules::InitPrintModule(lua_State *lua, World *world) {
    if (!world->GetScriptEngine()->ExecuteCode(InitPrint, "InitPrint")) {
        throw std::runtime_error("InitPrintModule execute code failed!");
    }

    void *v = world;
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Log")
            .addCClosure("Error", &ClosureWrap<World, &Lua_put<iConsole::LineType::Error>>, v)
            .addCClosure("Warning", &ClosureWrap<World, &Lua_put<iConsole::LineType::Warning>>, v)
            .addCClosure("Hint", &ClosureWrap<World, &Lua_put<iConsole::LineType::Hint>>, v)
            .addCClosure("Console", &ClosureWrap<World, &Lua_put<iConsole::LineType::Regular>>, v)
            .addCClosure("Debug", &ClosureWrap<World, &Lua_put<iConsole::LineType::Debug>>, v)
        .endNamespace()
        ;
}

}
