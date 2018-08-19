#include <pch.h>
#include <nfMoonGlare.h>

#include <Foundation/Scripts/LuaWrap.h>
#include <Foundation/Scripts/ErrorReporting.h>

#include "ScriptComponent.h"
#include "ScriptObject.h"
#include "GameObject.h"

namespace MoonGlare::Core::Scripts::Component {

ScriptObject::ScriptObject(lua_State *lua, InterfaceMap *world) {
    world->GetInterface<iRequireModule>()->RegisterRequire("ScriptComponent", this);
    InitMetatable(lua);
}

ScriptObject::~ScriptObject() {};

bool ScriptObject::OnRequire(lua_State *lua, std::string_view name) {
    lua_pushlightuserdata(lua, this);
    lua_gettable(lua, LUA_REGISTRYINDEX);
    return true;
};

//-------------------------------------------------------------------------------------------------

void ScriptObject::InitMetatable(lua_State *lua) {
    LuaStackOverflowAssert check(lua);

    lua_pushlightuserdata(lua, this);
    lua_gettable(lua, LUA_REGISTRYINDEX);

    if (!lua_isnil(lua, -1)) {
        lua_pop(lua, 1);
        return;
    }

    lua_pop(lua, 1);

    lua_createtable(lua, 0, 0);            // ScriptObjectMT
    lua_pushlightuserdata(lua, this);      // ScriptObjectMT this
    lua_pushvalue(lua, -2);                // ScriptObjectMT this ScriptObjectMT
    lua_settable(lua, LUA_REGISTRYINDEX);  // ScriptObjectMT

#ifdef DEBUG_SCRIPTAPI
    lua_pushvalue(lua, -1);
    lua_setglobal(lua, "ScriptObjectMetatable");
#endif          

    lua_pushvalue(lua, -1);
    lua_setfield(lua, -2, "__index");

    //lua_pushcfunction(lua, &ScriptObject::SetPerSecond);
    //lua_setfield(lua, -2, ScriptObject_SetPerSecond);

    lua_pushcfunction(lua, &ScriptObject::SetStep);
    lua_setfield(lua, -2, ScriptObject_SetStep);

    lua_pushcfunction(lua, &ScriptObject::SetActive);
    lua_setfield(lua, -2, ScriptObject_SetActive);

    lua_pushcfunction(lua, &ScriptObject::GetComponent);
    lua_setfield(lua, -2, ScriptObject_GetComponent);

    lua_pushcfunction(lua, &ScriptObject::SetInterval);
    lua_setfield(lua, -2, ScriptObject_SetInterval);

    lua_pushcfunction(lua, &ScriptObject::SetTimeout);
    lua_setfield(lua, -2, ScriptObject_SetTimeout);

    lua_pushcfunction(lua, &ScriptObject::KillTimer);
    lua_setfield(lua, -2, ScriptObject_KillTimer);

    lua_pop(lua, 1);
}

//-------------------------------------------------------------------------------------------------

bool ScriptObject::CallFunction(lua_State *lua, const char *funcName, int ArgC, int ErrFuncIndex) {
    ++ArgC;

    int selfIndex = GetAbsoluteindex(lua, -ArgC);

    //stack: Script arg0 ... argC

    lua_getfield(lua, selfIndex, funcName);                    //stack: Script arg0 ... argC func/nil

    if (lua_isnil(lua, -1)) {
        ReportMissingFunction(lua, funcName);
        lua_settop(lua, selfIndex - 1);
        return false; 
    }

    //stack: Script arg0 ... argC func

    lua_insert(lua, selfIndex);							         //stack: func Script arg0 ... argC
    LuaSafeCall(lua, ArgC, 0, funcName, ErrFuncIndex);
    return true;
}

//-------------------------------------------------------------------------------------------------

std::tuple<Entity, ScriptComponent*> ScriptObject::GetOwnerMembers(lua_State *lua, int selfIndex, const char *ScriptFunctionName) {
    lua_getfield(lua, selfIndex, ScriptInstance_Entity);
    if (!lua_islightuserdata(lua, -1)) {
        lua_pop(lua, 1);
        LuaRunError(lua, "Invalid component handle", "Internal __Entity field is invalid");
        return {  };
    }

    lua_getfield(lua, selfIndex, ScriptInstance_SubSystem);
    if (!lua_islightuserdata(lua, -1)) {
        lua_pop(lua, 2);
        LuaRunError(lua, "Invalid component handle", "Internal __SubSystem field is invalid");
        return {  };
    }

    Entity owner = Entity::FromVoidPtr(lua_touserdata(lua, -2));
    ScriptComponent *sc = (ScriptComponent*)lua_touserdata(lua, -1);
    lua_pop(lua, 2);
    return { owner, sc };
}

int ScriptObject::SetStep(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::SetStep";

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    auto[owner, componentPtr] = GetOwnerMembers(lua, 1, ScriptFunctionName);
    auto *entry = componentPtr->GetEntry(owner);
    if (!entry) {
        LuaRunError(lua, "Invalid component handle", "Internal component fields are invalid");
        return 0;
    }

    entry->m_Flags.m_Map.m_Step = lua_toboolean(lua, 2);

    return 0;
}

int ScriptObject::SetActive(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::SetActive";

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    auto[owner, componentPtr] = GetOwnerMembers(lua, 1, ScriptFunctionName);
    auto *entry = componentPtr->GetEntry(owner);
    if (!entry) {
        LuaRunError(lua, "Invalid component handle", "Internal component fields are invalid");
        return 0;
    }

    entry->m_Flags.m_Map.m_Active = lua_toboolean(lua, 2);
    return 0;
}

int ScriptObject::GetComponent(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::GetComponent";

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    auto[owner, componentPtr] = GetOwnerMembers(lua, 1, ScriptFunctionName);
    auto *entry = componentPtr->GetGameObject(owner);
    if (!entry) {
        LuaRunError(lua, "Invalid component handle", "Internal component fields are invalid");
        return 0;
    }

    return entry->GetComponent(lua);
}

int ScriptObject::SetInterval(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::SetInterval";

    int timerId = 0;
    float value = 0;

    switch (lua_gettop(lua)) {
    case 3:
        if (lua_type(lua, 3) != LUA_TNUMBER) {
            LuaRunError(lua, "Invalid argument #2", "Invalid type {}", lua_typename(lua, lua_type(lua, 3)));
            return 0;
        }
        timerId = lua_tointeger(lua, 3);        
        [[fallthrough]];
    case 2:
        if (lua_type(lua, 2) != LUA_TNUMBER) {
            LuaRunError(lua, "Invalid argument #1", "Invalid type {}", lua_typename(lua, lua_type(lua, 3)));
            return 0;
        }
        value = lua_tonumber(lua, 2);
        break;
    default:
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
        break;
    }

    auto[owner, componentPtr] = GetOwnerMembers(lua, 1, ScriptFunctionName);
    
    Handle h = componentPtr->GetTimerDispatcher().SetInterval(value/1000.0f, { owner, timerId });
    lua_pushlightuserdata(lua, h.GetVoidPtr());

    return 1;
}

int ScriptObject::SetTimeout(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::SetTimeout";

    int timerId = 0;
    float value = 0;

    switch (lua_gettop(lua)) {
    case 3:
        if (lua_type(lua, 3) != LUA_TNUMBER) {
            LuaRunError(lua, "Invalid argument #2", "Invalid type {}", lua_typename(lua, lua_type(lua, 3)));
            return 0;
        }
        timerId = lua_tointeger(lua, 3);
        [[fallthrough]];
    case 2:
        if (lua_type(lua, 2) != LUA_TNUMBER) {
            LuaRunError(lua, "Invalid argument #1", "Invalid type {}", lua_typename(lua, lua_type(lua, 3)));
            return 0;
        }
        value = lua_tonumber(lua, 2);
        break;
    default:
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
        break;
    }

    auto[owner, componentPtr] = GetOwnerMembers(lua, 1, ScriptFunctionName);

    Handle h = componentPtr->GetTimerDispatcher().SetTimeout(value / 1000.0f, { owner, timerId });
    lua_pushlightuserdata(lua, h.GetVoidPtr());

    return 1;
}

int ScriptObject::KillTimer(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::KillTimer";

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    if (lua_type(lua, 2) != LUA_TLIGHTUSERDATA) {
        LuaRunError(lua, "Invalid argument #1", "Invalid type {}", lua_typename(lua, lua_type(lua, 3)));
        return 0;
    }

    auto[owner, componentPtr] = GetOwnerMembers(lua, 1, ScriptFunctionName);

    Handle h = Handle::FromVoidPtr(lua_touserdata(lua, 2));
    componentPtr->GetTimerDispatcher().KillTimer(h);

    return 0;
}

}
