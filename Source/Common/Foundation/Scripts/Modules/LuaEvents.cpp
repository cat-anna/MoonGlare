#include "LuaEvents.h"

#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/ErrorReporting.h>

#include <Foundation/Scripts/LuaWrap.h>
#include <Foundation/Component/EventDispatcher.h>
#include <Foundation/Component/EventInfo.h>

namespace MoonGlare::Scripts::Modules {

LuaEventsModule::LuaEventsModule(lua_State *lua, InterfaceMap *world) {
    world->GetInterface<iRequireModule>()->RegisterRequire("Event", this);

    eventDispatcher = world->GetInterface<Component::EventDispatcher>();
    assert(eventDispatcher);
}

LuaEventsModule::~LuaEventsModule() {
}

//-------------------------------------------------------------------------------------------------

bool LuaEventsModule::OnRequire(lua_State *lua, std::string_view name) {
    luabridge::push<LuaEventsModule*>(lua, this);
    return true;
}

//-------------------------------------------------------------------------------------------------

/*@ [RequireModules/LuaEventsModule] Events module
    TODO @*/
ApiInitializer LuaEventsModule::RegisterScriptApi(ApiInitializer api) {

    //api.beginNamespace("Event")
    //    .RawLua([&](lua_State *lua) {
    //        int top = lua_gettop(lua);
    //        const char *t0 = lua_typename(lua, lua_type(lua, -1));

    //        int i = -1;
    //        luabridge::rawgetfield(lua, i, "SetSceneChangeFenceEvent");
    //        const char *t1 = lua_typename(lua, lua_type(lua, -1));
    //        lua_settop(lua, top);

    //       })
        //.endNamespace()
               //;
    //auto Class =
    return     api
        .beginClass<LuaEventsModule>("LuaEventsModule")
/*@ [LuaEventsModule/_] `Events:EmitEvent(event)`
    TODO @*/        
            .addFunction("Emit", &LuaEventsModule::EmitEvent)
        //;

/*@ [LuaEventsModule/PublicLuaEvents] Available events 
    This events can be created and send through events module @*/    
    //using BaseEventInfo = MoonGlare::Component::BaseEventInfo;
    //BaseEventInfo::ForEachEvent([&](auto evId, auto &evInfo) {
    //    if (evInfo.apiInitFunc && evInfo.isPublic)
//        Class.RawLua([&](lua_State *lua) {
//            int top = lua_gettop(lua);
//            luaL_dostring(lua,
//R"==(
//for k,v in pairs(api) do
//    print(k, v)
//end
//print(api, api.Event)
//return api
//)==");
//
//            int top2 = lua_gettop(lua);
//            const char *t = lua_typename(lua, lua_type(lua, -1));
//            //lua_getglobal(lua, "api");
//            //if (lua_isnil(lua, -1)) {
//            //    lua_settop(lua, top);
//            //    return;
//            //}
//            //lua_getfield(lua, -1, "Event");
//            if (lua_isnil(lua, -1)) {
//                lua_settop(lua, top);
//                return;
//            }
//            luabridge::rawsetfield(lua, (-2) + (-1), "Event");

    //        int top = lua_gettop(lua);
    //        luabridge::lua_rawgetp(lua, LUA_REGISTRYINDEX, evInfo.infoPtr->GetClassStaticKey());
    //        if (lua_isnil(lua, -1)) {
    //            lua_pop(lua, 1);
    //            //TODO: error
    //        } else {
    //            luabridge::rawgetfield(lua, -1, "__call");
    //            if (lua_isnil(lua, -1)) {
    //                lua_pop(lua, 1);
    //                //TODO: error
    //            } else {
    //                int top_ = lua_gettop(lua);
    //                lua_insert(lua, -2);
    //                lua_pop(lua, 1);
    //                luabridge::rawsetfield(lua, (-2) + (-1), evInfo.EventName);
    //            }
    //        }
            //int top2 = lua_gettop(lua);
        //});
    //});

    //return Class
        .endClass();
}

/*@ [DebugApi/LuaEventsModuleDebug] Events module debug api @*/
ApiInitializer LuaEventsModule::RegisterDebugScriptApi(ApiInitializer api) {
    struct T {
        static int AllEvents(lua_State *lua) {
            lua_createtable(lua, 0, 0);
            Component::BaseEventInfo::ForEachEvent([lua](Component::EventClassId evid, const Component::BaseEventInfo::EventClassInfo& info) {
                lua_pushinteger(lua, static_cast<int>(evid));
                lua_setfield(lua, -2, info.EventName);
            });
            return 1;
        }

        static int SetEventLogs(lua_State *lua) {
            static constexpr char *ScriptFunctionName = "debug.SetEventLogs";
            auto index = lua_tointeger(lua, 1);
            if (index < 0 || index >= (int)Component::BaseEventInfo::GetUsedEventTypes()) {
                LuaReportInvalidArg(lua, 1, integer);
                return 0;
            }
            bool v = lua_toboolean(lua, 2);

            auto &info = Component::BaseEventInfo::GetEventTypeInfo((Component::EventClassId)index);
            info.infoPtr->GetLogsEnabled() = v;
            return 0;
        }
    };
    return api
/*@ [LuaEventsModuleDebug/_] debug.AllEvents()
    Returns table with all events. Key is event name, value is event id @*/
        .addCFunction("AllEvents", &T::AllEvents)
/*@ [LuaEventsModuleDebug/_] debug.SetEventLogs(eventId, value)
    Enable logs for specific event @*/
        .addCFunction("SetEventLogs", &T::SetEventLogs)
        ;
}

//-------------------------------------------------------------------------------------------------

int LuaEventsModule::EmitEvent(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "Event::Emit";

    if (lua_gettop(lua) != 2) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    //int type0 = lua_type(lua, -1);
    //const char *name0 = lua_typename(lua, type0);

    if (!lua_isuserdata(lua, -1)) {
        LuaRunError(lua, "Invalid argument #1", "Invalid argument type: {}", lua_typename(lua, lua_type(lua, 2)));
        return 0;
    }

    lua_getfield(lua, -1, "EventId");
    int type = lua_type(lua, -1);
    //const char *name = lua_typename(lua, type);
    if (type != LUA_TNUMBER) {
        lua_pop(lua, 1);
        __debugbreak();
        LuaRunError(lua, "Invalid argument #1", "Argument is not a valid event object");
        return 0;
    }
    auto eid = lua_tointeger(lua, -1);
    lua_pop(lua, 1);

#ifdef DEBUG_LOG
    lua_getfield(lua, -1, "EventName");
    const char *evName = lua_tostring(lua, -1);
    AddLog(Debug, "Emitting event: %s", evName);
    lua_pop(lua, 1);
#endif // DEBUG_LOG

    auto &evInfo = Component::BaseEventInfo::GetEventTypeInfo(static_cast<Component::EventClassId>(eid));

    auto success = evInfo.queueFromLua(lua, 2, eventDispatcher);
    if (!success) {
        LuaRunWarning(lua, "Event emission failed", "");
    }

    return 0;
}            

} //namespace MoonGlare::Core::Scripts::Modules
