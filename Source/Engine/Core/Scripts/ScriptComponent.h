#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/LuaStackOverflowAssert.h>
#include <Foundation/Scripts/iLuaRequire.h>

#include <Foundation/Component/EntityEvents.h>
#include <Foundation/Component/EntityArrayMapper.h>
#include <Foundation/Component/EventDispatcher.h>

#include <Foundation/TimerDispatcher.h>

#include <Memory/StaticVector.h>

#include <Core/Scripts/ScriptEngine.h>

namespace MoonGlare::Core::Scripts::Component {
using namespace MoonGlare::Scripts;
using namespace Core::Component;

struct GameObjectTable;
struct ScriptObject;

class ScriptComponent
    : public iSubsystem
    , public SubSystemIdWrap<SubSystemId::Script> {
public:
    friend struct GameObject;    //TODO: this is temporary

    ScriptComponent(SubsystemManager *owner);
    virtual ~ScriptComponent();

    bool Initialize() override;
    bool Finalize() override;
    int PushToLua(lua_State *lua, Entity owner) override;
    void Step(const SubsystemUpdateData &conf) override;
    bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

    void HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event);
    void HandleEvent(lua_State* lua, Entity destination);           

    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
            bool m_Active : 1; //Script has step function and it shall be called

            bool m_Step : 1;
        };
        MapBits_t m_Map;
        uint32_t m_UintValue;

        void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
        void ClearAll() { m_UintValue = 0; }

        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
    };

    struct ScriptEntry {
        FlagsMap m_Flags;
        Entity m_OwnerEntity;	

        void Reset() {
            m_Flags.m_Map.m_Valid = false;
        }
    };
    //static_assert((sizeof(ScriptEntry) % 8) == 0, "Invalid ScriptEntry size!");
    //static_assert(std::is_pod<ScriptEntry>::value, "ScriptEntry must be pod!");

    int GetGameObject(lua_State *lua, Entity Owner);
    GameObject* GetGameObject(Entity Owner);

    ScriptEntry* GetEntry(Entity e) { 
        auto index = m_EntityMapper.GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return nullptr;
        return &m_Array[index];
    }

    EventScriptSink* GetEventSink() { return &eventScriptSinkProxy; }

    struct TimerData {
        Entity owner;
        int timerId;
    };
    auto &GetTimerDispatcher() { return timerDispatcher; } //TODO: this is ugly

    static ApiInitializer RegisterScriptApi(ApiInitializer root);
protected:
    SubsystemManager *subSystemManager;
    Scripts::ScriptEngine *m_ScriptEngine;
    iRequireModule *requireModule;                                         
    std::shared_ptr<GameObjectTable> gameObjectTable;
    ScriptObject *scriptObject;
    EventScriptSinkProxy<ScriptComponent> eventScriptSinkProxy{ this };

    template<class T> using Array = Memory::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;
    Array<ScriptEntry> m_Array;
    EntityArrayMapper<> m_EntityMapper;
    TimerDispatcher<TimerData, 1024> timerDispatcher;

    void ReleaseComponent(lua_State *lua, size_t Index);

    void GetInstancesTable(lua_State *lua) {
        lua_pushlightuserdata(lua, GetInstancesTablePointer());
        lua_gettable(lua, LUA_REGISTRYINDEX);
    }
    void *GetInstancesTablePointer() { return this; }
    SubsystemManager* GetManager() { return subSystemManager; }
private:
//support functions
    //int lua_GetScriptComponent(lua_State *lua, Entity Owner);
    int lua_GetComponentInfo(lua_State *lua, SubSystemId cid, Entity Owner);
    static int lua_DereferenceHandle(lua_State *lua);
    static int lua_SetComponentState(lua_State *lua);
}; 											

} //namespace MoonGlare::Core::Scripts::Component 
