#pragma once

#include <pugixml.hpp>
#include <boost/noncopyable.hpp>

#include "../InterfaceMap.h"
#include "Entity.h"
#include "nfComponent.h"

namespace MoonGlare::Component {

struct SubsystemUpdateData {
    double timeDelta = 0.0f;
    double globalTime = 0.0f;
};

class iSubsystemManager : private boost::noncopyable {
public:
    virtual ~iSubsystemManager() {}
    virtual InterfaceMap& GetInterfaceMap() = 0;
    virtual EventDispatcher& GetEventDispatcher() = 0;
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
