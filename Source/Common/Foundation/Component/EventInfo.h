#pragma once

#include <typeinfo>
#include <string>

#include "Configuration.h"

#include <Foundation/Scripts/ApiInit.h>

namespace MoonGlare::Component {

using EventClassId = uint32_t;

class BaseEventInfo {
public:
    static EventClassId GetUsedEventTypes() { return idAlloc; }

    virtual const std::type_info &GetTypeInfo() const = 0;

	struct EventClassInfo {
        size_t byteSize;
        Scripts::ApiInitFunc apiInitFunc;
        const BaseEventInfo *infoPtr = nullptr;
        const char *EventName;
        const char *HandlerName;
#ifdef DEBUG
        bool pod;
#endif
	};

	static void Dump(std::ostream &output);
    static const EventClassInfo& GetEventTypeInfo(EventClassId id) { return GetEventClassesTypeInfo()[id]; }

    using EventClassesTypeTable = std::array<EventClassInfo, Configuration::MaxEventTypes>;
protected:
	template<class T>
    static EventClassId AllocateEventClass();
    static EventClassesTypeTable& GetEventClassesTypeInfo();
private:
	static EventClassId AllocateId() { return idAlloc++; }
	static EventClassId idAlloc;
};

template<class T>
struct EventInfo : public BaseEventInfo {
	static EventClassId GetClassId() { return classId; }
//	static_assert(std::is_pod<T>::value, "Event must be pod type!");
    const std::type_info &GetTypeInfo() const override { return typeid(T); }
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
    assert(id < Configuration::MaxEventTypes);
    GetEventClassesTypeInfo()[id] = {
        sizeof(T),
        Scripts::GetApiInitFunc<T>(),
        &t,
        T::EventName,
        T::HandlerName,
#ifdef DEBUG
        std::is_pod<T>::value,
#endif
    };
    return id;
}

} 
