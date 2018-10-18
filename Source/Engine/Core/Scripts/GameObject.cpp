#include <pch.h>
#include <nfMoonGlare.h>

#include <Foundation/Component/EntityManager.h>
#include <Foundation/Component/ComponentEvents.h>

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Scripts/LuaWrap.h>
#include <Foundation/Scripts/ErrorReporting.h>

#include "GameObject.h"
#include "ScriptComponent.h"
#include <Core/Component/ComponentRegister.h>

#include "../EntityBuilder.h"

namespace MoonGlare::Core::Scripts::Component {   

//-------------------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer GameObject::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
    .beginClass<GameObject>("GameObject")
        .addCFunction("GetComponent", &GameObject::GetComponent)
        .addCFunction("CreateComponent", &GameObject::CreateComponent)
        .addCFunction("LoadObject", &GameObject::LoadObject)
        .addFunction("Destroy", &GameObject::Destroy)
        .addFunction("GetName", &GameObject::GetName)
        .addFunction("SetName", &GameObject::SetName)

        .addProperty("Name", &GameObject::GetName, &GameObject::SetName)

        .addData(TagFieldName, &GameObject::Tag)
    .endClass()
    ;
}

//-------------------------------------------------------------------------------------------------

int GameObject::CreateComponent(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "GameObject::CreateComponent";

    LuaStackOverflowAssert check(lua);
    SubSystemId ssid = SubSystemId::Invalid;

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    switch (lua_type(lua, 2)) {
    case LUA_TNUMBER:
        ssid = static_cast<SubSystemId>(lua_tointeger(lua, 2));
        break;
    case LUA_TSTRING:
        if (ComponentRegister::GetComponentID(lua_tostring(lua, 2), ssid))
            break;
        LuaRunError(lua, "Invalid argument #1", "Unknown component: {}", lua_tostring(lua, 2));
    default:
        LuaRunError(lua, "Invalid argument #1", "Invalid argument type: {}", lua_typename(lua, lua_type(lua, 2)));
        return 0;
    }

    //stack: self ssid

    if (!entityManager->IsValid(owner)) {
        LuaRunError(lua, "Invalid GameObject", "GameObject is not valid");
        return 0;
    }

    if (ssid < SubSystemId::CoreBegin) {
        auto &carr = myWorld->GetComponentArray();
        carr.Create(owner, static_cast<ComponentClassId>(ssid));
        int r = carr.PushToScript(owner, static_cast<ComponentClassId>(ssid), lua);
        return check.ReturnArgs(r);
    }

    auto *cptr = myWorld->GetComponent(ssid);
    if (!cptr) {
        LuaRunError(lua, "Invalid Component Id", "There is no component with id: {}", (int)ssid);
        return 0;
    }

    if (cptr->Create(owner)) {
        myWorld->GetEventDispatcher().Send(ComponentCreatedEvent{ owner, (uint32_t)ssid });
        lua_pushboolean(lua, true);
        return check.ReturnArgs(1);
    } else {
        LuaRunError(lua, "Component creation failed", "component id: {}", (int)ssid);
        lua_pushnil(lua);
        return check.ReturnArgs(1);
    }
}

int GameObject::GetComponent(lua_State * lua) {
    static constexpr char *ScriptFunctionName = "GameObject::GetComponent";
    LuaStackOverflowAssert check(lua);
    SubSystemId ssid = SubSystemId::Invalid;

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    switch (lua_type(lua, 2)) {
    case LUA_TNUMBER:
        ssid = static_cast<SubSystemId>(lua_tointeger(lua, 2));
        break;
    case LUA_TSTRING:
        if (ComponentRegister::GetComponentID(lua_tostring(lua, 2), ssid))
            break;
        LuaRunError(lua, "Invalid argument #1", "Unknown component: {}", lua_tostring(lua, 2));
    default:
        LuaRunError(lua, "Invalid argument #1", "Invalid argument type: {}", lua_typename(lua, lua_type(lua, 2)));
        return 0;
    }

    if (ssid < SubSystemId::CoreBegin) {
        auto &carr = myWorld->GetComponentArray();
        int r = carr.PushToScript(owner, static_cast<ComponentClassId>(ssid), lua);
        return check.ReturnArgs(r);
    }


    auto ssptr = myWorld->GetComponent(ssid);
    if (!ssptr) {
        LuaRunError(lua, "Cannot find requested subsystem", "There is no subsystem {}", (int)ssid);
        return 0;
    }

    int r = ssptr->PushToLua(lua, owner);
    //TODO: if (r==0) raise error . Can be done when scriptComponent->lua_GetComponentInfo is no more 
    if (r > 0 || ssid == SubSystemId::Script)
        return check.ReturnArgs(r);

    return check.ReturnArgs(scriptComponent->lua_GetComponentInfo(lua, ssid, owner));
}

//-------------------------------------------------------------------------------------------------

int GameObject::LoadObject(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "GameObject::LoadObject";

    /*
    GameObject:LoadObject {
    Object = "URI",
    [Name = "..."]
    [Tag = "..."]

    [Transform = { ... },]
    [RectTransform = { ... },]

     --TODO:
     data passed to script OnCreate function
     }

    returns:
        GameObject
     */

    LuaStackOverflowAssert check(lua);
    int argc = lua_gettop(lua);

    if (argc != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", argc - 1);
        return 0;
    }

    //stack: self arg

    lua_getfield(lua, 2, "Object");									        //stack: self arg arg.objectURI		
    const char *objectURI = lua_tostring(lua, -1);
    if (!objectURI) {
        lua_settop(lua, argc);
        LuaRunError(lua, "Invalid object URI", "Object URI is not a valid string");
        return 0;
    }

    lua_getfield(lua, 2, "Name");										   //stack: self arg arg.objectURI arg.objectName		
    const char *objectName = lua_tostring(lua, -1);

    lua_getfield(lua, 2, TagFieldName);										   //stack: self arg arg.objectURI arg.objectName arg.Tag		
    std::optional<int> tagValue;
    if (lua_isnumber(lua, -1))
        tagValue = static_cast<int>(lua_tonumber(lua, -1));
    lua_pop(lua, 1);

    //stack: self arg arg.objectURI arg.objectName	

    Entity child;
    if (!EntityBuilder::Build(myWorld, owner, objectURI, child, (objectName ? std::string(objectName) : std::string()))) {
        lua_settop(lua, argc);
        LuaRunError(lua, "Load object failed!", "");
        return 0;
    }

    lua_pop(lua, 2);													//stack: self

    AddLog(Hint, ScriptFunctionName << " Loaded: " << child << " parent:" << owner << " Object:" << (objectName ? objectName : "[NULL]"));

    lua_pushnil(lua);
    while (lua_next(lua, 2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        if (lua_type(lua, -2) != LUA_TSTRING) {
            LuaRunWarning(lua, "Invalid key type", "Only strings are accepted, got {}", lua_typename(lua, lua_type(lua, 2)));
            lua_pop(lua, 1);
            continue;
        }
        const char *key = lua_tostring(lua, -2);

        switch (Space::Utils::MakeHash32(key)) {
        case "Name"_Hash32:   //ignored keys
        case "Tag"_Hash32:
        case "Object"_Hash32:
            lua_pop(lua, 1);
            continue;
        default:
            break;
        }

        int absValue = lua_gettop(lua) + (-1) + 1;

        SubSystemId ssid;
        if (!ComponentRegister::GetComponentID(key, ssid)) {
            LuaRunWarning(lua, "Unknown component name", "Name '{}' does not match any known component", key);
            lua_pop(lua, 1);
            continue;
        }

        auto ssptr = myWorld->GetComponent(ssid);
        if (!ssptr) {
            LuaRunWarning(lua, "Component does not exists", "Wanted component '{}' does not exists in current context", key);
            lua_pop(lua, 1);
            continue;
        }

        if (ssptr->PushToLua(lua, child) != 1) {
            LuaRunWarning(lua, "Object does not have requested component", "Object does not have requested component '{}'", key);
            lua_pop(lua, 1);
            continue;
        }

        int absComponent = lua_gettop(lua) + (-1) + 1;

        lua_pushnil(lua);
        while (lua_next(lua, absValue) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            lua_pushvalue(lua, -2);
            lua_insert(lua, -2);
            lua_settable(lua, absComponent);
        }

        lua_pop(lua, 2);
    }

    lua_settop(lua, argc);

    scriptComponent->GetGameObject(lua, child);

    if (tagValue.has_value()) {
        lua_pushinteger(lua, tagValue.value());
        lua_setfield(lua, -2, TagFieldName);
    }

    return check.ReturnArgs(1);
}

//-------------------------------------------------------------------------------------------------

void GameObject::Destroy() {
    static constexpr char *ScriptFunctionName = "GameObject::Destroy";

    if (!entityManager->Release(owner)) {
        LuaRunWarning(nullptr, "Destroy failed", "");
    }
}

//-------------------------------------------------------------------------------------------------

void GameObject::SetName(const char *name) {
    static constexpr char *ScriptFunctionName = "GameObject::SetName";
    if (!entityManager->SetEntityName(owner, name ? name : "")) {
        LuaRunWarning(nullptr, "SetName failed", "");
    }
}

const char* GameObject::GetName() const {
    static constexpr char *ScriptFunctionName = "GameObject::GetName";

    const std::string *name = nullptr;
    if (!entityManager->GetEntityName(owner, name) || !name) {
        LuaRunWarning(nullptr, "GetName failed", "");
        return "";
    }

    return name->c_str();
}

//-------------------------------------------------------------------------------------------------

}
