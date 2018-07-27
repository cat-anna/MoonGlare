#pragma once

#include "nfComponent.h"
#include "Configuration.h"
#include "EventInfo.h"

//#include <Core/Scripts/ScriptEngine.h>

namespace MoonGlare::Component {

class EventScriptSink {
public:
    //event data will be on top of the stack
    virtual void HandleEvent(lua_State* lua, Entity Destination) = 0;
};

struct BaseEventCallDispatcher {
    template<typename EVENT>
    void Dispatch(const EVENT &ev) {
        AddLog(Event, "Dispatching event: " << ev << " recipients:" << m_Handlers.size());

        for (auto &handler : m_Handlers)
            handler.Call(&ev);
    }

    template<typename RECIVER, typename EVENT, void(RECIVER::*HANDLER)(const EVENT&)>
    void AddHandler(RECIVER *reciver) {
        struct F {
            static void Call(void *reciver, const void *event) {
                ASSERT(reciver);
                ASSERT(event);
                auto *r = reinterpret_cast<RECIVER*>(reciver);
                const EVENT &ev = *reinterpret_cast<const EVENT*>(event);
                (r->*HANDLER)(ev);
            }
        };
        for (auto &e : m_Handlers)
            if (e.m_Reciver == reciver)
                return; // already registered;
        m_Handlers.emplace_back( HandlerCaller { reciver, &F::Call });
    }
protected:
    using HandlerFunction = void(*)(void *reciver, const void *event);
    struct HandlerCaller {
        void *m_Reciver;
        HandlerFunction m_Function;
        void Call(const void * event) { m_Function(m_Reciver, event); }
    };
    std::vector < HandlerCaller > m_Handlers;
};

class EventDispatcher {
public:
    EventDispatcher();
    
    void SetEventSink(lua_State *lua, EventScriptSink *sink);

    template<typename EVENT>
    void Send(const EVENT& event) {
        auto classid = EventInfo<EVENT>::GetClassId();
        assert(classid < Configuration::MaxEventTypes);
        eventDispatchers[classid].Dispatch(event);
    }

    template<typename EVENT>
    void SendTo(const EVENT& event, Entity recipient) {
        AddLog(Event, "Dispatching event: " << event << " recipient:" << recipient);
        auto classid = EventInfo<EVENT>::GetClassId();
        assert(classid < Configuration::MaxEventTypes);
        //m_EventDispatchers[classid].Dispatch(event, recipient);
        SendToScript(event, recipient);
    }

    template<typename EVENT, typename RECIVER, void(RECIVER::*HANDLER)(const EVENT&)>
    void Register(RECIVER *reciver) {
        auto classid = EventInfo<EVENT>::GetClassId();
        assert(classid < Conf::MaxEventTypes);
        eventDispatchers[classid].AddHandler<RECIVER, EVENT, HANDLER>(reciver);
    }
    template<typename EVENT, typename RECIVER>
    void Register(RECIVER *reciver) {
        return Register < EVENT, RECIVER, static_cast<void(RECIVER::*)(const EVENT&)>(&RECIVER::HandleEvent) > (reciver);
    }
    template<typename EVENT, typename RECIVER>
    void RegisterTemplate(RECIVER *reciver) {
        return Register < EVENT, RECIVER, &RECIVER::HandleEventTemplate<EVENT>>(reciver);
    }
private:
    template<typename T>
    using Array = std::array<T, Configuration::MaxEventTypes>;

    template<typename EVENT>
    void SendToScript(const EVENT& event, Entity recipient) {
        if (luaState && eventSink) {
            luabridge::push(luaState, event);
            eventSink->HandleEvent(luaState, recipient);
        }
    }

    Array<BaseEventCallDispatcher> eventDispatchers;
    lua_State *luaState;
    EventScriptSink *eventSink;
};

}
