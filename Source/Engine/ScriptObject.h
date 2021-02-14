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

/*@ [ScriptObjectOverridable/_] ScriptObject::OnCreate()
    This function is called before entity destruction. [TODO: Is it called at all?]   
    ScriptComponent is destructed as first one, so it is safe to access other component in this method.
    Function should not return anything.
@*/
    //Calls OnCreate. Script shall be on top, pops it from stack
    static void OnCreate(lua_State *lua, int ErrFuncIndex) { 
        CallFunction(lua, ScriptObject_OnCreate, 0, ErrFuncIndex); 
    }
/*@ [ScriptObjectOverridable/_] ScriptObject::OnDestroy()
    This function is called after entity construction.   
    ScriptComponent is initialized as last one, so it is safe to access other component in this method.
    Function should not return anything.
@*/    
    //Calls OnDestroy. Script shall be on top, pops it from stack
    static void OnDestroy(lua_State *lua, int ErrFuncIndex) {
        CallFunction(lua, ScriptObject_OnDestroy, 0, ErrFuncIndex);
    }

/*@ [ScriptObjectOverridable/_] ScriptObject::Step(stepData)
    This function is called every engine loop.
    TODO: stepData
    Function should not return anything
@*/
    //Calls Step. Script shall be on top, pops it from stack
    static bool Step(lua_State *lua, int MoveCfgIndex, int ErrFuncIndex) {
        lua_pushvalue(lua, MoveCfgIndex);
        return CallFunction(lua, ScriptObject_Step, 1, ErrFuncIndex);
    }

/*@ [ScriptObjectEventHandler/_] ScriptObject::OnTimer(cookie, timerHandle)
    This function is called when one of timers is expired. Single shot timers are removed automatically.
    Cookie value is the same value as passed to one of timer setup metods.
    If you want to remove periodic timer here pass timerHandle to KillTimer method.
    Function should not return anything.
@*/
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
