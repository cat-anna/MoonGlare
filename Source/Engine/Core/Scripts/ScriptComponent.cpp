#include <pch.h>
#include <MoonGlare.h>

#include <Foundation/Component/ComponentEvents.h>
#include <Foundation/Component/EntityManager.h>

#include <Core/Component/TemplateStandardComponent.h>
#include "ScriptComponent.h"
#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/EntityBuilder.h>

#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/LuaWrap.h>
#include <Foundation/Scripts/LuaStackOverflowAssert.h>
#include <Core/Scripts/ScriptEngine.h>
#include <Core/Scripts/LuaApi.h>

#include <Core/Component/TransformComponent.h>

#include <ScriptComponent.x2c.h>

#include "GameObject.h"
#include "ScriptObject.h"

namespace MoonGlare::Core::Scripts::Component {
using namespace MoonGlare::Scripts;

namespace lua {
    enum {
        SelfPtrUpValue = 1,
        EntityUpValue,
    };

//Script instance callable functions
    static const char *Function_Step = "Step";
    static const char *Function_OnCreate = "OnCreate";
    static const char *Function_OnDestroy = "OnDestroy";
    static const char *Function_PerSecond = "PerSecond";

//TODO: check:
    static const char *DereferenceHandleName = "Get";
    static const char *ComponentSetStateName = "Set";
}				    

::Space::RTTI::TypeInfoInitializer<ScriptComponent, ScriptComponent::ScriptEntry> ScriptComponentTypeInfo;
RegisterComponentID<ScriptComponent> ScriptComponent("Script", true);

static ScriptObject *gScriptObject = nullptr;  //TODO: this is ugly!

ScriptComponent::ScriptComponent(SubsystemManager *Owner)
    : AbstractSubsystem(Owner) {
}

ScriptComponent::~ScriptComponent() {
}

ApiInitializer ScriptComponent::RegisterScriptApi(ApiInitializer root) {
    return GameObject::RegisterScriptApi(root);
}

bool ScriptComponent::Initialize() {
    auto &ed = GetManager()->GetEventDispatcher();
    ed.Register<Component::EntityDestructedEvent>(this);

    m_Array.MemZeroAndClear();
    m_EntityMapper.Fill(ComponentIndex::Invalid);

    m_ScriptEngine = GetManager()->GetWorld()->GetScriptEngine();
    THROW_ASSERT(m_ScriptEngine, "No script engine instance!");
    auto lua = m_ScriptEngine->GetLua();
    LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);

    LuaStackOverflowAssert check(lua);

    lua_pushlightuserdata(lua, GetInstancesTablePointer());									 //stack: InstTblIdx
    lua_createtable(lua, m_Array.Capacity(), 0);											 //stack: InstTblIdx InstTbl
    PublishSelfLuaTable(lua, "ScriptComponent", this, -1);							         //stack: InstTblIdx InstTbl
    lua_settable(lua, LUA_REGISTRYINDEX);													 //stack: -

    requireModule = m_ScriptEngine->QuerryModule<iRequireModule>();
    if (!requireModule) {
        AddLogf(Error, "QuerryModule<iRequireModule> failed!");
        return false;
    }

    gameObjectTable = GetManager()->GetWorld()->GetSharedInterface<GameObjectTable>();
    if (!gameObjectTable) { //TODO: this is ugly!
        gameObjectTable = std::make_shared<GameObjectTable>();
        gameObjectTable->Clear();
        GetManager()->GetWorld()->SetSharedInterface<GameObjectTable>(gameObjectTable);
    }
    
    if (!gScriptObject) {
        gScriptObject = new ScriptObject();
        gScriptObject->Init(lua, requireModule);
    }

    return true;
}

bool ScriptComponent::Finalize() {
    auto lua = m_ScriptEngine->GetLua();
    LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
    HideSelfLuaTable(lua, "ScriptComponent", this);
    lua_pushlightuserdata(lua, (void *)this);
    lua_pushnil(lua);
    lua_settable(lua, LUA_REGISTRYINDEX);
    return true;
}

//-------------------------------------------------------------------------------------------------

void ScriptComponent::HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event) {
    auto index = m_EntityMapper.GetIndex(event.entity);
    if (index == ComponentIndex::Invalid)
        return;

    auto &item = m_Array[index];
    item.m_Flags.m_Map.m_Valid = false;
    gameObjectTable->Release(event.entity);
}

void ScriptComponent::HandleEvent(lua_State * L, Entity destination) {
    if (!GetManager()->GetWorld()->GetEntityManager().IsValid(destination))
        return;
    auto *entry = GetEntry(destination);
    if (!entry || !entry->m_Flags.m_Map.m_Valid || !entry->m_Flags.m_Map.m_Active 
        //|| !entry->m_Flags.m_Map.m_Event
        ) {
        return;
    }

    int index = m_EntityMapper.GetIndex(destination);

    LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
    LuaStackOverflowAssert check(L);

    int luatop = lua_gettop(L);

    //stack: eventObj

    lua_getfield(L, -1, "HandlerName");
    if (lua_isnil(L, -1)) {
        //TODO
        __debugbreak();
        lua_settop(L, luatop);
        return;
    }
    
    const char *HandlerName = lua_tostring(L, -1);
    
    lua_pushcclosure(L, LuaErrorHandler, 0);                    //stack: eventObj HandlerName errH
    int errf = lua_gettop(L);

    GetInstancesTable(L);									    //stack: eventObj HandlerName errH instT
    lua_rawgeti(L, -1, index);							        //stack: eventObj HandlerName errH instT Script/nil

    if (!lua_istable(L, -1)) {
        lua_settop(L, luatop);
        AddLogf(Error, "ScriptComponent: nil in lua script table at index: %d", index);
        //TODO: change message and channel to SCRT
        return;
    }

    //stack: eventObj HandlerName errH instT Script
    lua_pushvalue(L, -4);                                       //stack: eventObj HandlerName errH instT Script HandlerName 
    lua_gettable(L, -2);		                                //stack: eventObj HandlerName errH instT Script func/nil

    if (lua_isnil(L, -1)) {
        AddLogf(Warning, "ScriptComponent: There is no %s function in component at index: %d", HandlerName, index);
        lua_settop(L, luatop);
        //TODO: change message and channel to SCRT; no message?
        //entry->m_Flags.m_Map.m_Event = false;
        return;
    }

    // stack: eventObj HandlerName errH instT Script func
    lua_insert(L, -2);							             //stack: eventObj HandlerName errH instT func Script 
    lua_pushvalue(L, -6);							         //stack: eventObj HandlerName errH instT func Script eventObj       

    if (!LuaSafeCall(L, 2, 0, HandlerName, errf)) {
        AddLogf(Error, "Failure during %s call for component #%lu", HandlerName, index);
        //TODO: change message and channel to SCRT
    }

    lua_settop(L, luatop);         //clear all temp values on stack
    lua_pop(L, 1);                 //remove event object
}

//-------------------------------------------------------------------------------------------------

void ScriptComponent::Step(const MoveConfig & conf) {
    if (m_Array.empty()) {
        return;
    }

    auto lua = m_ScriptEngine->GetLua();
    LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
    LuaStackOverflowAssert check(lua);
    //stack: -		

    lua_pushcclosure(lua, LuaErrorHandler, 0);
    int errf = lua_gettop(lua);

    GetInstancesTable(lua);									//stack: self
    luabridge::Stack<const MoveConfig*>::push(lua, &conf);  //stack: self movedata

    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

    int luatop = lua_gettop(lua);

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {
        auto &item = m_Array[i];
        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and ignore
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }

        if (!item.m_Flags.m_Map.m_Active) 
            continue; // entry is not active. nothing todo.
        
        if (!((item.m_Flags.m_Map.m_OnPerSecond && conf.m_SecondPeriod) || item.m_Flags.m_Map.m_Step))
            continue; // there is no function which can be called. nothing todo.

        lua_rawgeti(lua, -2, i + 1);		//stack: self movedata Script/nil

        if (!lua_istable(lua, -1)) {
            lua_settop(lua, luatop);
            AddLogf(Error, "ScriptComponent: nil in lua script table at index: %d", i + 1);
            continue;
        }

        int luascrtop = lua_gettop(lua);
            
        if (item.m_Flags.m_Map.m_Step) {
            lua_pushvalue(lua, -1);								// stack: self movedata script script
            lua_getfield(lua, -1, lua::Function_Step);			//stack: self movedata script script Step/nil
            if (lua_isnil(lua, -1)) {
                item.m_Flags.m_Map.m_Step = false;
                AddLogf(Warning, "ScriptComponent: There is no Step function in component at index: %d, disabling", i);
            } else {
                //stack: self movedata script script Step
                lua_insert(lua, -2);							//stack: self movedata script Step script
                lua_pushvalue(lua, -4);							//stack: self movedata script Step script movedata

                if (!LuaSafeCall(lua, 2, 0, lua::Function_Step, errf)) {
                    AddLogf(Error, "Failure during OnStep call for component #%lu", i);
                }
            }
        }

        if (item.m_Flags.m_Map.m_OnPerSecond && conf.m_SecondPeriod) {
            lua_settop(lua, luascrtop);							//stack: self movedata script 
            lua_pushvalue(lua, -1);								//stack: self movedata script script
            lua_getfield(lua, -1, lua::Function_PerSecond);		//stack: self movedata script script persec/nil
            if (lua_isnil(lua, -1)) {
                item.m_Flags.m_Map.m_OnPerSecond = false;
                AddLogf(Warning, "ScriptComponent: There is no PerSecond function in component at index: %d, disabling", i);
            } else {
                //stack: self movedata script script persec
                lua_insert(lua, -2);						//stack: self movedata script persec script 
                if (!LuaSafeCall(lua, 1, 0, lua::Function_Step, errf)) {
                    AddLogf(Error, "Failure during PerSecond call for component #%lu", i);
                }
            }
        }

        lua_settop(lua, luatop);
    }

    lua_pop(lua, 3); //stack: -

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "ScriptComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        ReleaseComponent(lua, LastInvalidEntry);
    }
}

void ScriptComponent::ReleaseComponent(lua_State *lua, size_t Index) {
    LuaStackOverflowAssert check(lua);

    auto lastidx = m_Array.Allocated() - 1;
    auto LuaIndex = Index + 1;				//lua index
    auto LuaLast = m_Array.Allocated();		//lua index

    GetInstancesTable(lua);						//stack: InstancesTable
    lua_pushinteger(lua, LuaIndex);				//stack: InstancesTable LuaIndex
    lua_gettable(lua, -2);						//stack: InstancesTable Script

    if (lua_isnil(lua, -1)) {
        AddLogf(Error, "ScriptComponent: nil in lua Object script table at index: 1");
        lua_pop(lua, 1);							    //stack: InstancesTable 
    } else {
        lua_getfield(lua, -1, lua::Function_OnDestroy); //stack: InstancesTable Script OnDestroy/nil
        if (lua_isnil(lua, -1)) {
            lua_pop(lua, 2);                            //stack: InstancesTable 
        } else {
            lua_pushcclosure(lua, LuaErrorHandler, 0);  //stack: InstancesTable Script OnDestroy errH
            lua_insert(lua, -3);                        //stack: InstancesTable errH Script OnDestroy 
            lua_pushvalue(lua, -2);			            //stack: InstancesTable errH Script OnDestroy Script
            if (!LuaSafeCall(lua, 1, 0, lua::Function_OnDestroy, -3)) {
                //	nothing there, nothing more to be logged
            }
            //stack: InstancesTable errH Script
            lua_pop(lua, 2);                            //stack: InstancesTable 
        }
    }

    //stack: InstancesTable

    ScriptEntry *entry = nullptr;
    if (lastidx == Index) {
        entry = &m_Array[lastidx];
        m_EntityMapper.ClearIndex(entry->m_OwnerEntity);
    } else {
        auto &last = m_Array[lastidx];
        auto &item = m_Array[Index];
        entry = &last;

        std::swap(last, item);
        m_EntityMapper.Swap(last.m_OwnerEntity, item.m_OwnerEntity);

        //set last in place of current and nil last
        lua_pushinteger(lua, LuaLast);			//stack: InstancesTable LuaLast
        lua_gettable(lua, -2);					//stack: InstancesTable last_table
        lua_pushinteger(lua, LuaIndex);			//stack: InstancesTable last_table LuaIndex
        lua_insert(lua, -2);					//stack: InstancesTable LuaIndex last_table 
        lua_settable(lua, -3);					//stack: InstancesTable 
    }

    //set last in place of current and nil last
    lua_pushinteger(lua, LuaLast);			//stack: InstancesTable LuaLast
    lua_pushnil(lua);						//stack: InstancesTable LuaLast nil
    lua_settable(lua, -3);					//stack: InstancesTable 

    lua_pop(lua, 1); //stack: -

    entry->Reset();
    m_Array.DeallocateLast();
}

//-------------------------------------------------------------------------------------------------

bool ScriptComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {

    //if(gameObjectTable->IsUsed(owner)) TODO:

    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();

    x2c::Component::ScriptComponent::ScriptEntry_t se;
    se.ResetToDefault();
    if (!reader.Read(se)) {
        AddLog(Error, "Failed to read ScriptEntry!");
        return false;
    }

    auto lua = m_ScriptEngine->GetLua();
    LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
    LuaStackOverflowAssert check(lua);

    int top = lua_gettop(lua);
    //stack: -

    lua_pushcclosure(lua, LuaErrorHandler, 0);
    int errf = lua_gettop(lua);

    if (se.m_Script[0] == '/') {
        if (!requireModule->Querry(lua, se.m_Script)) {
            AddLogf(Error, "There is no such script: '%s'", se.m_Script.c_str());
            //no need to deallocate entry. It will be handled by internal garbage collecting mechanism
            lua_pop(lua, 1);
            return false;
        }
    } else {
        AddLogf(Error, "There is no such script: '%s'", se.m_Script.c_str());
        lua_pop(lua, 1);
        return false;
    }

    //stack: ScriptClass

    entry.m_OwnerEntity = owner;
    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_Step = true;
    entry.m_Flags.m_Map.m_Active = se.m_Active;
    entry.m_Flags.m_Map.m_OnPerSecond = se.m_PerSecond;
    //entry.m_Flags.m_Map.m_Event = true;
    m_EntityMapper.SetIndex(owner, index);

    auto *gameObject = gameObjectTable->Set(owner, GetManager(), this, &GetManager()->GetWorld()->GetEntityManager());
    if (!gameObject) {
        //TODO
    }

    //stack: ScriptClass ObjectRoot GameObject 

    lua_createtable(lua, 0, 0);											//stack: ScriptClass Script
    lua_insert(lua, -2);                                        		//stack: Script ScriptClass 
    lua_setmetatable(lua, -2);                                  		//stack: Script

    luabridge::push<GameObject*>(lua, gameObject);              		//stack: Script gameObject
    lua_setfield(lua, -2, ScriptObject::ScriptInstance_GameObject);		//stack: Script

    lua_pushlightuserdata(lua, owner.GetVoidPtr());				        //stack: Script Entity
    lua_setfield(lua, -2, ScriptObject::ScriptInstance_Entity);			//stack: Script

    lua_pushlightuserdata(lua, this);				            		//stack: Script SubSystem
    lua_setfield(lua, -2, ScriptObject::ScriptInstance_SubSystem);		//stack: Script

    GetInstancesTable(lua);												//stack: Script InstTable
    lua_pushvalue(lua, -2);												//stack: Script InstTable Script
    lua_rawseti(lua, -2, static_cast<int>(index) + 1);					//stack: Script InstTable
    lua_pop(lua, 1);

    //stack: Script
                                                                         
    lua_getfield(lua, -1, lua::Function_OnCreate);				//stack: Script OnCreate/nil
    if (lua_isnil(lua, -1)) {
        lua_pop(lua, 1);										//stack: Script
    } else {
        lua_pushvalue(lua, -2);									//stack: Script OnCreate Script
        if (!LuaSafeCall(lua, 1, 0, lua::Function_OnCreate, errf)) {
            //no need for more logging
        }
        //stack: Script result
        lua_pop(lua, 1);										
    }

    //stack: errf Script 
    lua_settop(lua, top);

    //stack: -
    return true;
}

//-------------------------------------------------------------------------------------------------

GameObject* ScriptComponent::GetGameObject(Entity Owner) {
    auto *ptr = gameObjectTable->GetObject(Owner);
    if (!ptr) {
        //TODO: handle this case properly
        ptr = gameObjectTable->Set(Owner, GetManager(), this, &GetManager()->GetWorld()->GetEntityManager());
        if (!ptr) {
            //TODO
        }
    }
    return ptr;
}

int ScriptComponent::GetGameObject(lua_State *lua, Entity Owner) {
    auto ptr = GetGameObject(Owner);
    if (!ptr)
        return 0;
    luabridge::push<GameObject*>(lua, ptr);
    return 1;
}

//-------------------------------------------------------------------------------------------------

int ScriptComponent::lua_GetScriptComponent(lua_State *lua, Entity Owner) {
    auto index = m_EntityMapper.GetIndex(Owner);
    if (index == ComponentIndex::Invalid)
        return 0;
    
    auto &e = m_Array[index];
    if (!e.m_Flags.m_Map.m_Valid) {
        return 0;
    }

    int luatop = lua_gettop(lua);
    LuaStackOverflowAssert check(lua);

    //stack: -
    GetInstancesTable(lua);				//stack: insttable
    lua_rawgeti(lua, -1, index + 1);		//stack: insttable script/nil

    if (!lua_istable(lua, -1)) {
        lua_settop(lua, luatop);
        return 0;
    }
    
    lua_insert(lua, -2);
    lua_pop(lua, 1);

    return check.ReturnArgs(1);
}

int ScriptComponent::lua_GetComponentInfo(lua_State *lua, SubSystemId cid, Entity Owner) {

    auto cptr = GetManager()->GetComponent(cid);
    if (!cptr) {
        AddLogf(Error, "GetComponent: Error: There is no component %d", cid);
        return 0;
    }

    int r = cptr->PushToLua(lua, Owner);
    if (r > 0) {
        return r;
    }

    return lua_MakeComponentInfo(lua, cid, Owner, cptr);
}

int ScriptComponent::lua_MakeComponentInfo(lua_State *lua, SubSystemId cid, Entity owner, iSubsystem *cptr) {
    LuaStackOverflowAssert check(lua);

    lua_createtable(lua, 0, 5);

    lua_pushlightuserdata(lua, cptr);
    lua_pushlightuserdata(lua, owner.GetVoidPtr());
    lua_pushcclosure(lua, &lua_DereferenceHandle, 2);
    lua_setfield(lua, -2, lua::DereferenceHandleName);

    lua_pushlightuserdata(lua, cptr);
    lua_pushlightuserdata(lua, owner.GetVoidPtr());
    lua_pushcclosure(lua, &lua_SetComponentState, 2);
    lua_setfield(lua, -2, lua::ComponentSetStateName);

    if (m_ScriptEngine->GetComponentEntryMT(cid)) {
        lua_setmetatable(lua, -2);
    }

    return check.ReturnArgs(1);
}

int ScriptComponent::lua_DereferenceHandle(lua_State *lua) {
    void *voidcptr = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
    iSubsystem *cptr = reinterpret_cast<iSubsystem*>(voidcptr);

    Entity e = Entity::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::EntityUpValue)));

    int rets = 0;
    if (!cptr->PushEntryToLua(e, lua, rets)) {
        AddLogf(Error, "ComponentInstanceInfo::Get: Error: Component '%s' does not support lua api", typeid(*cptr).name());
        return 0;
    }

    return rets;
}

int ScriptComponent::lua_SetComponentState(lua_State *lua) {
    int argc = lua_gettop(lua);
    if (argc != 2) {
        AddLogf(Error, "ComponentInstanceInfo::Set: Error: Invalid argument count!");
        return 0;
    }

    if (lua_type(lua, -1) != LUA_TTABLE) {
        AddLogf(Error, "ComponentInstanceInfo::Set: Error: Invalid argument type!");
        return 0;
    }

    //stack: self valtable
    LuaStackOverflowAssert check(lua);

    void *voidcptr = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
    iSubsystem *cptr = reinterpret_cast<iSubsystem*>(voidcptr);

    Entity e = Entity::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::EntityUpValue)));
    int rets = 0;
    if (!cptr->PushEntryToLua(e, lua, rets)) {
        lua_pushvalue(lua, -2);
        AddLog(Error, "ComponentInstanceInfo::Set: Error: Component '" << typeid(*cptr).name() << "' does not support lua api owner:" << e );
        lua_settop(lua, argc);
        return 0;
    } 

    if (rets == 0) {
        lua_settop(lua, argc);
        return 0;
    }

    //stack: self values component/self

    lua_pushnil(lua);								//stack: self values component nil

    while (lua_next(lua, 2) != 0) {					//stack: self values component key value
        lua_pushvalue(lua, -2);						//stack: self values component key value key
        lua_insert(lua, -2);						//stack: self values component key key value 
        lua_settable(lua, 3);						//stack: self values component key  
    }
    //stack: self values component/self
    lua_pop(lua, 1);

    return 0;
}

} //namespace MoonGlare::Core::Scripts::Component  
