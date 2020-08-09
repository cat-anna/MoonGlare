#pragma once

#include <typeinfo>
#include <string>

#include <boost/tti/has_member_data.hpp>
#include <boost/tti/has_static_member_data.hpp>

#include "Configuration.h"
#include "Entity.h"

#include <Foundation/Scripts/ApiInit.h>

namespace MoonGlare::Component {

namespace detail {
BOOST_TTI_HAS_MEMBER_DATA(recipient)
BOOST_TTI_HAS_STATIC_MEMBER_DATA(logsEnabled)
}

class EventDispatcher;

enum class EventClassId : uint16_t { Invalid = 0, };

class BaseEventInfo {
public:
    static EventClassId GetUsedEventTypes() { return static_cast<EventClassId>(idAlloc+1); }

    virtual const std::type_info &GetTypeInfo() const = 0;
	virtual bool& GetLogsEnabled() const {
		static bool v = false;
		return v;
	};
    virtual Scripts::ClassKey GetClassKey() const = 0;
    virtual Scripts::ClassKey GetClassStaticKey() const = 0;
    virtual Scripts::ClassKey GetClassConstKey() const = 0;

    using QueueFromLuaFunc = bool(*)(lua_State *lua, int index, EventDispatcher *dispatcher);
    using GetFromLuaFunc = const void*(*)(lua_State *lua, int index);

	struct EventClassInfo {
        EventClassId id = EventClassId::Invalid;
        size_t byteSize = 0;
        Scripts::ApiInitFunc apiInitFunc = nullptr;
        QueueFromLuaFunc queueFromLua = nullptr;
        GetFromLuaFunc getFromLuaFunc = nullptr;
        const BaseEventInfo *infoPtr = nullptr;
        const char *EventName;
        const char *HandlerName;
        bool isPublic;
        bool pod;
        bool hasRecipient;
	};

	static void Dump(std::ostream &output);

    static const EventClassInfo& GetEventTypeInfo(uint32_t id) {
        return GetEventClassesTypeInfo()[id];
    }
    static const EventClassInfo& GetEventTypeInfo(EventClassId id) {
        return GetEventTypeInfo(static_cast<uint32_t>(id));
    }

    template<typename FUNC>
    static void ForEachEvent(FUNC && func) {
        for (EventClassId i = (EventClassId)1; i < GetUsedEventTypes(); i = (EventClassId)((int)i + 1))
            func(i, GetEventTypeInfo(i));
    }

    template<typename T>
    static const void* GetFromLua(lua_State *lua, int index) {
        return luabridge::Stack<T*>::get(lua, index);
    }
    template<typename T>
    static bool QueueFromLua(lua_State *lua, int index, EventDispatcher *dispatcher) {
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

    using EventClassesTypeTable = std::array<EventClassInfo, Configuration::MaxEventTypes>;
protected:
	template<class T>
    static EventClassId AllocateEventClass();
    static EventClassesTypeTable& GetEventClassesTypeInfo();
private:
	static EventClassId AllocateId() { return static_cast<EventClassId>(++idAlloc); }
	static std::underlying_type_t<EventClassId> idAlloc;
};

template<class T>
struct EventInfo : public BaseEventInfo {
//	static_assert(std::is_pod<T>::value, "Event must be pod type!");

	static EventClassId GetClassId() { return classId; }

    using HasRecipient = detail::has_member_data_recipient<T, Entity>;
    
    const std::type_info &GetTypeInfo() const override { return typeid(T); }

#ifdef DEBUG_SCRIPTAPI
	static bool logsEnabled;
	bool& GetLogsEnabled() const override { return logsEnabled; }
#else
	static constexpr bool logsEnabled = false;
#endif

    virtual Scripts::ClassKey GetClassKey() const { return luabridge::ClassInfo<T>::getClassKey();}
    virtual Scripts::ClassKey GetClassStaticKey() const { return luabridge::ClassInfo<T>::getStaticKey(); }
    virtual Scripts::ClassKey GetClassConstKey() const { return luabridge::ClassInfo<T>::getConstKey(); }
private:
	static const EventClassId classId;
};

#ifdef DEBUG_SCRIPTAPI
template<class T>
bool EventInfo<T>::logsEnabled = detail::has_static_member_data_logsEnabled<T, bool>::value;
#endif

template<typename T>
const EventClassId EventInfo<T>::classId = BaseEventInfo::AllocateEventClass<T>();

template<typename T>
EventClassId EventClassIdValue = EventInfo<T>::GetClassId();

template<class T>
EventClassId BaseEventInfo::AllocateEventClass() {
    auto id = AllocateId();
    static const EventInfo<T> t;
    assert((size_t)id < Configuration::MaxEventTypes);

    //size_t typeSize = sizeof(T);
    //size_t alignment = std::alignment_of_v<T>;

    GetEventClassesTypeInfo()[(size_t)id] = {
        id,
        sizeof(T),
        Scripts::GetApiInitFunc<T>(),
        &QueueFromLua<T>,
        &GetFromLua<T>,
        &t,
        T::EventName,
        T::HandlerName,
        T::Public,
        std::is_pod<T>::value,
        EventInfo<T>::HasRecipient::value,
    };
    return id;
}

} 
