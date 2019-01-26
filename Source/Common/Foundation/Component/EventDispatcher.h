#pragma once

#include "Configuration.h"
#include "EventInfo.h"    
#include "nfComponent.h"

#include <Foundation/Memory/DynamicBuffer.h>

namespace MoonGlare::Component {
    
class EventScriptSink {
public:
    //event data will be on top of the stack
    virtual void HandleEvent(lua_State* lua, Entity Destination) = 0;
};

template<typename T>
class EventScriptSinkProxy : public EventScriptSink {
    T *reciver;
public:
    EventScriptSinkProxy(T *r) : reciver(r) {}
    void HandleEvent(lua_State* lua, Entity destination) override {
        reciver->HandleEvent(lua, destination);
    }
};

struct BaseEventCallDispatcher {
    template<typename EVENT>
    void Dispatch(const EVENT &ev) {
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
    std::vector<HandlerCaller> m_Handlers;
};

class EventDispatcher {
public:
    EventDispatcher();
    void Step();
    void SetEventSink(lua_State *lua, EventScriptSink *sink);

    template<typename EVENT>
    void Send(const EVENT& event) {
		if (EventInfo<EVENT>::logsEnabled) {
			AddLog(Event, "Dispatching event: " << event);
		}
        auto classid = EventInfo<EVENT>::GetClassId();
        assert((uint32_t)classid < Configuration::MaxEventTypes);
        eventDispatchers[(size_t)classid].Dispatch(event);      
        if constexpr (EventInfo<EVENT>::HasRecipient::value) {
            SendToScript(event, event.recipient);
        }
        for (auto *disp : subDispatcher)
            disp->Send(event);
    }

    template<typename EVENT>
    void Queue(const EVENT& event) {
		if (EventInfo<EVENT>::logsEnabled) {
			AddLog(Event, "Queued event: " << event);
		}
		std::lock_guard<std::recursive_mutex> lock(bufferMutex);
        auto *buf = buffer.Allocate<QueuedEvent<EVENT>>();
        buf->event = event;
        buf->sendFunc = reinterpret_cast<BaseQueuedEvent::SendFunc>(&EventDispatcher::SendQueuedEvent<EVENT>);
    }

    template<typename EVENT, typename RECIVER, void(RECIVER::*HANDLER)(const EVENT&)>
    void Register(RECIVER *reciver) {
        auto classid = EventInfo<EVENT>::GetClassId();
        assert((size_t)classid < Configuration::MaxEventTypes);
        eventDispatchers[(size_t)classid].AddHandler<RECIVER, EVENT, HANDLER>(reciver);
    }
    template<typename EVENT, typename RECIVER>
    void Register(RECIVER *reciver) {
        return Register<EVENT, RECIVER, static_cast<void(RECIVER::*)(const EVENT&)>(&RECIVER::HandleEvent)>(reciver);
    }

    void AddSubDispatcher(MoonGlare::Component::EventDispatcher* ed) {
        std::lock_guard<std::recursive_mutex> lock(bufferMutex);
        subDispatcher.insert(ed);
    }
    void RemoveSubDispatcher(MoonGlare::Component::EventDispatcher* ed) {
        std::lock_guard<std::recursive_mutex> lock(bufferMutex);
        subDispatcher.erase(ed);
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

    struct BaseQueuedEvent {
        using SendFunc = void(EventDispatcher::*)(const BaseQueuedEvent&);
        SendFunc sendFunc;
    };

    template<typename EVENT>
    struct QueuedEvent : public BaseQueuedEvent {
        EVENT event;
    };
    
    template<typename EVENT>
    void SendQueuedEvent(const QueuedEvent<EVENT> &qev) { Send(qev.event); }

    Array<BaseEventCallDispatcher> eventDispatchers;
    lua_State *luaState = nullptr;
    EventScriptSink *eventSink = nullptr;
    std::recursive_mutex bufferMutex;
    std::set<EventDispatcher*> subDispatcher;
    Memory::DynamicBuffer<Configuration::EventDispatcherQueueSize> buffer;
};

}
