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

    lua_pushcfunction(lua, &ScriptObject::SetPerSecond);
    lua_setfield(lua, -2, ScriptObject_SetPerSecond);

    lua_pushcfunction(lua, &ScriptObject::SetStep);
    lua_setfield(lua, -2, ScriptObject_SetStep);

    lua_pushcfunction(lua, &ScriptObject::SetActive);
    lua_setfield(lua, -2, ScriptObject_SetActive);

    lua_pushcfunction(lua, &ScriptObject::GetComponent);
    lua_setfield(lua, -2, ScriptObject_GetComponent);

    lua_pop(lua, 1);
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

int ScriptObject::SetPerSecond(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "ScriptComponent::SetPerSecond";

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

    entry->m_Flags.m_Map.m_OnPerSecond = lua_toboolean(lua, 2);
    return 0;
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

}