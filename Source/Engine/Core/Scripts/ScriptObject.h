#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <Foundation/Handle.h>


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
    static constexpr char *ScriptObject_SetInterval = "SetInterval";
    static constexpr char *ScriptObject_SetTimeout = "SetTimeout";
    static constexpr char *ScriptObject_KillTimer = "KillTimer";


    static constexpr char *ScriptObject_OnCreate = "OnCreate";
    static constexpr char *ScriptObject_OnDestroy = "OnDestroy";
    static constexpr char *ScriptObject_Step = "Step";
    static constexpr char *ScriptObject_OnTimer = "OnTimer";

    static constexpr char *RequireName = "ScriptComponent";

    ScriptObject(lua_State *lua, InterfaceMap *world);
    ~ScriptObject() ;

    bool OnRequire(lua_State *lua, std::string_view name) override;


    //Calls OnCreate. Script shall be on top, pops it from stack
    static void OnCreate(lua_State *lua, int ErrFuncIndex) { 
        CallFunction(lua, ScriptObject_OnCreate, 0, ErrFuncIndex); 
    }
    //Calls OnDestroy. Script shall be on top, pops it from stack
    static void OnDestroy(lua_State *lua, int ErrFuncIndex) {
        CallFunction(lua, ScriptObject_OnDestroy, 0, ErrFuncIndex);
    }

    //Calls Step. Script shall be on top, pops it from stack
    static bool Step(lua_State *lua, int MoveCfgIndex, int ErrFuncIndex) {
        lua_pushvalue(lua, MoveCfgIndex);
        return CallFunction(lua, ScriptObject_Step, 1, ErrFuncIndex);
    }

    //Calls Step. Script shall be on top, pops it from stack
    static bool OnTimer(lua_State *lua, int timerId, Handle timerH, int ErrFuncIndex) {
        lua_pushinteger(lua, timerId);
        lua_pushlightuserdata(lua, timerH.GetVoidPtr());
        return CallFunction(lua, ScriptObject_OnTimer, 2, ErrFuncIndex);
    }

    /*
      Calls script object function
      Requires top of stack as: Script Arg0...ArgC
      Requires absolute ErrFuncIndex value
      ArgC shall not include self
      CallFunction always pops all arguments
      returns false only if there is no such function
    */
    static bool CallFunction(lua_State *lua, const char *funcName, int ArgC, int ErrFuncIndex);
protected:
    void InitMetatable(lua_State *lua);
    
    static std::tuple<Entity, ScriptComponent*> GetOwnerMembers(lua_State *lua, int selfIndex, const char *ScriptFunctionName);
    static int SetStep(lua_State * lua);
    static int SetActive(lua_State * lua);
    static int GetComponent(lua_State * lua);
    static int SetInterval(lua_State * lua);
    static int SetTimeout(lua_State * lua);
    static int KillTimer(lua_State * lua);
};

}
