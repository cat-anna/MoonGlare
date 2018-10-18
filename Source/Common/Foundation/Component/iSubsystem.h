#pragma once

#include <pugixml.hpp>
#include <boost/noncopyable.hpp>

#include <Foundation/TimeUtils.h>

#include "../InterfaceMap.h"
#include "Entity.h"
#include "nfComponent.h"

#include "ComponentArray.h"

namespace MoonGlare::Component {

struct SubsystemUpdateData {
    using TimePoint = std::chrono::steady_clock::time_point;

    double timeDelta = 0.0;
    double globalTime = 0.0;
    double localTime = 0.0;

    TimePoint globalTimeStart;
    TimePoint currentTime;
    TimePoint localTimeStart;
    double localTimeBase = 0.0;

    void UpdateTime(TimePoint point) {
        timeDelta = TimeDiff(currentTime, point);
        globalTime = TimeDiff(globalTimeStart, point);
        localTime = TimeDiff(localTimeStart, point) + localTimeBase;
        currentTime = point;
    }
    void ResetTime(TimePoint point) {
        currentTime = point;
        globalTimeStart = point;
        localTimeStart = point;
    }
};

class iSubsystemManager : private boost::noncopyable {
public:
    virtual ~iSubsystemManager() {}
    virtual InterfaceMap& GetInterfaceMap() = 0;
    virtual EventDispatcher& GetEventDispatcher() = 0;
    virtual ComponentArray& GetComponentArray() = 0;
};

struct ComponentReader {
    pugi::xml_node node;

    template<typename T>
    bool Read(T &t) {
        return t.Read(node);
    }
};

class iSubsystem : private boost::noncopyable {
public:
    virtual ~iSubsystem() {}

    using ComponentIndex = MoonGlare::Component::ComponentIndex;
    using Entity = MoonGlare::Component::Entity;

    virtual bool Initialize() { return true; }
    virtual bool Finalize() { return true; }
    virtual void Step(const SubsystemUpdateData &conf) { }
    virtual int PushToLua(lua_State *lua, Entity owner) { return 0; };
    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) { return true; }
    virtual bool Create(Entity owner) { return false; }

    //TODO: remove old interface

    virtual bool LoadComponentConfiguration(pugi::xml_node node) { return true; }
    virtual bool PushEntryToLua(Entity owner, lua_State *lua, int &luarets) { return false; }
protected:
    iSubsystem() {}
};

using UniqueSubsystem = std::unique_ptr<iSubsystem>;

}
