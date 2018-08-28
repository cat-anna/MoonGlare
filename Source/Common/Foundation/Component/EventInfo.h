#pragma once

#include <typeinfo>
#include <string>

#include "Configuration.h"

#include <Foundation/Scripts/ApiInit.h>

namespace MoonGlare::Component {

class EventDispatcher;

enum class EventClassId : uint16_t { Invalid = 0xFFFF, };

class BaseEventInfo {
public:
    static EventClassId GetUsedEventTypes() { return static_cast<EventClassId>(idAlloc); }

    virtual const std::type_info &GetTypeInfo() const = 0;
    virtual const void* GetFromLua(lua_State *lua, int index) const = 0;
    virtual bool QueueFromLua(lua_State *lua, int index, EventDispatcher *dispatcher) const = 0;

	struct EventClassInfo {
        size_t byteSize;
        Scripts::ApiInitFunc apiInitFunc;
        const BaseEventInfo *infoPtr = nullptr;
        const char *EventName;
        const char *HandlerName;
        bool isPublic;
        bool pod;
	};

	static void Dump(std::ostream &output);

    static const EventClassInfo& GetEventTypeInfo(uint32_t id) {
        return GetEventClassesTypeInfo()[id];
    }
    static const EventClassInfo& GetEventTypeInfo(EventClassId id) {
        return GetEventTypeInfo(static_cast<uint32_t>(id));
    }

    using EventClassesTypeTable = std::array<EventClassInfo, Configuration::MaxEventTypes>;
protected:
	template<class T>
    static EventClassId AllocateEventClass();
    static EventClassesTypeTable& GetEventClassesTypeInfo();
private:
	static EventClassId AllocateId() { return static_cast<EventClassId>(idAlloc++); }
	static std::underlying_type_t<EventClassId> idAlloc;
};

template<class T>
struct EventInfo : public BaseEventInfo {
//	static_assert(std::is_pod<T>::value, "Event must be pod type!");

	static EventClassId GetClassId() { return classId; }
    
    const std::type_info &GetTypeInfo() const override { return typeid(T); }

    const void* GetFromLua(lua_State *lua, int index) const override {
        return luabridge::Stack<T*>::get(lua, index);
    }
    bool QueueFromLua(lua_State *lua, int index, EventDispatcher *dispatcher) const override {
        if constexpr (std::is_trivial_v<T>) {
            T* event = luabridge::Stack<T*>::get(lua, index);
            if (!event) {
                AddLogf(Error, "Invalid event type at lua stack at index %d", index);
                return false;
            }
            dispatcher->Queue(*event);
            return true;
        } else {
            return false;
        }
    }

private:
	static const EventClassId classId;
};

template<typename T>
const EventClassId EventInfo<T>::classId = BaseEventInfo::AllocateEventClass<T>();

template<typename T>
EventClassId EventClassIdValue = EventInfo<T>::GetClassId();

template<class T>
EventClassId BaseEventInfo::AllocateEventClass() {
    auto id = AllocateId();
    static const EventInfo<T> t;
    assert((size_t)id < Configuration::MaxEventTypes);
    GetEventClassesTypeInfo()[(size_t)id] = {
        sizeof(T),
        Scripts::GetApiInitFunc<T>(),
        &t,
        T::EventName,
        T::HandlerName,
        T::Public,
        std::is_pod<T>::value,
    };
    return id;
}

} 
