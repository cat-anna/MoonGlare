#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Scripts/iLuaRequire.h>

namespace MoonGlare::Core::Scripts::Component {
using namespace MoonGlare::Core::Component;
using namespace MoonGlare::Component;
using namespace MoonGlare::Scripts;

class ScriptComponent;

struct ScriptObject : public iDynamicScriptModule, public iRequireRequest {
    static constexpr char *ScriptInstance_SubSystem = "__SubSystem";
    static constexpr char *ScriptInstance_Entity = "__Entity";
    static constexpr char *ScriptInstance_GameObject = "GameObject";

    static constexpr char *ScriptObject_SetPerSecond = "SetPerSecond";
    static constexpr char *ScriptObject_SetStep = "SetStep";
    static constexpr char *ScriptObject_SetActive = "SetActive";
    static constexpr char *ScriptObject_GetComponent = "GetComponent";

    static constexpr char *RequireName = "ScriptComponent";

    ScriptObject(lua_State *lua, InterfaceMap *world);
    ~ScriptObject() ;

    bool OnRequire(lua_State *lua, std::string_view name);
protected:
    void InitMetatable(lua_State *lua);
    
    static std::tuple<Entity, ScriptComponent*> GetOwnerMembers(lua_State *lua, int selfIndex, const char *ScriptFunctionName);
    static int SetPerSecond(lua_State *lua);
    static int SetStep(lua_State * lua);
    static int SetActive(lua_State * lua);
    static int GetComponent(lua_State * lua);
};

}
