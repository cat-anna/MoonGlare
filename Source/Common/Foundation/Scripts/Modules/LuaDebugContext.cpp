#include <Foundation/InterfaceMap.h>
#include <Foundation/Scripts/LuaStackOverflowAssert.h>

#include "../ApiInit.h"
#include "../iDynamicModule.h"
#include "../iLuaRequire.h"

#include <Foundation/Scripts/ExecuteCode.h>
#include <Foundation/Scripts/LuaWrap.h>

#include <Foundation/Module/iDebugContext.h>

#include "LuaDebugContext.h"

namespace MoonGlare::Scripts::Modules {

struct CallableObject {
    Module::iDebugContext::DebugCommand command;

    int Call(lua_State *lua) {
        try {
            if (command)
                command();
        }
        catch (const std::exception &e) {
            AddLogf(Error, "Exception during debug command: %s", e.what());
        }
        catch (...) {
            AddLogf(Error, "Unknown exception during debug command");
        }
        return 0;
    }
};

LuaDebugContextModule::LuaDebugContextModule(lua_State *lua, InterfaceMap *world) : interfaceMap(*world) {
    LuaStackOverflowAssert overflowtest(lua);
    lua_getglobal(lua, "debug");
    if (lua_isnil(lua, -1)) {
        lua_pop(lua, 1);
    } else {
        lua_PushCClosure(lua, &LuaDebugContextModule::IndexMethod, (void*)this);
        lua_setfield(lua, -2, "__index");
        lua_pushvalue(lua, -1);
        lua_setmetatable(lua, -2);
        lua_pop(lua, 1);
    }
}

LuaDebugContextModule::~LuaDebugContextModule() {}

ApiInitializer LuaDebugContextModule::RegisterScriptApi(ApiInitializer api) {
    return api
        .beginClass<CallableObject>("CallableObject")
            .addCFunction("__call", &CallableObject::Call)
        .endClass()
        ;
}

ApiInitializer LuaDebugContextModule::RegisterDebugScriptApi(ApiInitializer api) {
    return api;
}

int LuaDebugContextModule::IndexMethod(lua_State *lua) {
    int debugIdx = 1;
    int nameIdx = 2;
    LuaStackOverflowAssert overflowtest(lua);

    const char *name = lua_tostring(lua, nameIdx);

    lua_rawget(lua, debugIdx);
    if (!lua_isnil(lua, -1)) {
        return overflowtest.ReturnArgs(1);
    }

    lua_pop(lua, 1);
    lua_pushvalue(lua, nameIdx);

    void* This = lua_touserdata(lua, lua_upvalueindex(1));
    LuaDebugContextModule *ctx = reinterpret_cast<LuaDebugContextModule*>(This);

    auto debugCtx = ctx->interfaceMap.GetSharedInterface<Module::iDebugContext>();

    if(!debugCtx) {
        AddLogf(Warning, "Cannot get debug context");
        return overflowtest.ReturnArgs(0);
    }

    auto &cmds = debugCtx->GetCommandMap();

    auto it = cmds.find(name);
    if (it == cmds.end()) {
        AddLogf(Warning, "There is no command %s", name);
        return overflowtest.ReturnArgs(0);
    }

    CallableObject co;
    co.command = it->second;
    luabridge::Stack<CallableObject>::push(lua, co);

    return overflowtest.ReturnArgs(1);
}

} //namespace MoonGlare::Scripts::Modules
