#include <pch.h>
#include <nfMoonGlare.h>
#include <iConsole.h>

#include "../ScriptEngine.h"
#include "StaticModules.h"

namespace MoonGlare::Core::Scripts::Modules {

#include <LuaPrint.lua.h>

template<iConsole::LineType lt>
static int Lua_put(lua_State *L, iConsole *c) {
    const char *msg = luaL_checkstring(L, -1);
    if (!msg || *msg == '\0')
        return 0;
    bool toConsole = true;
    switch (lt) {
    case MoonGlare::iConsole::LineType::Error:
        AddLog(Error, msg);
        break;
    case MoonGlare::iConsole::LineType::Warning:
        AddLog(Warning, msg);
        break;
    case MoonGlare::iConsole::LineType::Hint:
        AddLog(Hint, msg);
        break;
    case MoonGlare::iConsole::LineType::Debug:
        toConsole = false;
        AddLog(Debug, msg);
        break;
    default:
        AddLog(Info, msg);
        break;
    }                   
    if (toConsole && c) {
        c->AddLine(msg, lt);
    }
    return 0;
}

template<typename T, int(*F)(lua_State*, iConsole* ptr)>
int ClosureWrap(lua_State* lua) {
    T *t = reinterpret_cast<T*>(lua_touserdata(lua, lua_upvalueindex(1)));
    return F(lua, t);
}

void StaticModules::InitPrint(lua_State *lua, World *world) {
    DebugLogf(Debug, "Initializing Print module");

    if (!world->GetScriptEngine()->ExecuteCode((const char*)LuaPrint_lua, LuaPrint_lua_size, "InitPrint")) {
        throw std::runtime_error("InitPrintModule execute code failed!");
    }

    iConsole *c;
    world->GetObject(c);
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Log")
            .addCClosure("Error", &ClosureWrap<iConsole, &Lua_put<iConsole::LineType::Error>>, (void*)c)
            .addCClosure("Warning", &ClosureWrap<iConsole, &Lua_put<iConsole::LineType::Warning>>, (void*)c)
            .addCClosure("Hint", &ClosureWrap<iConsole, &Lua_put<iConsole::LineType::Hint>>, (void*)c)
            .addCClosure("Console", &ClosureWrap<iConsole, &Lua_put<iConsole::LineType::Regular>>, (void*)c)
            .addCClosure("Debug", &ClosureWrap<iConsole, &Lua_put<iConsole::LineType::Debug>>, (void*)c)
        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
