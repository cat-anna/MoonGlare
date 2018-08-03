#pragma once

#include <pugixml.hpp>

#include "../InterfaceMap.h"
#include "Entity.h"
#include "nfComponent.h"

namespace MoonGlare::Component {

enum class SubsystemId : uint8_t {
    Invalid = 0xFF,
};

struct SubsystemUpdateData {
    double timeDelta = 0.0f;
    double globalTime = 0.0f;
};

class iSubsystemManager {
public:
    virtual ~iSubsystemManager() {}
    virtual InterfaceMap& GetInterfaceMap() = 0;
    virtual ComponentArray& GetComponentArray() = 0;
    virtual EventDispatcher& GetEventDispatcher() = 0;
};

class iSubsystem {
public:
    virtual ~iSubsystem() {}

    virtual void Update(const SubsystemUpdateData &conf) { } 

    //todo: remove old interface

    virtual bool Initialize() { return true; }
    virtual bool Finalize() { return true; }

    virtual bool Load(pugi::xml_node node, Entity Owner, Handle &hout) { return true; }
    virtual bool GetInstanceHandle(Entity Owner, Handle &hout) { return false; }

    virtual bool Create(Entity Owner, Handle &hout) {
        AddLogf(Error, "Abstract function called! class: %s", typeid(*this).name());
        return false;
    }

    virtual bool LoadComponentConfiguration(pugi::xml_node node) { return true; }
    virtual bool PushEntryToLua(Handle h, lua_State *lua, int &luarets) { return false; }

    //virtual void OnEntityDestroyed(Entity e) { }
    //virtual void OnEntityCrated(Entity e) { }
protected:
    iSubsystem() {}
private:
};

using UniqueSubsystem = std::unique_ptr<iSubsystem>;

}
