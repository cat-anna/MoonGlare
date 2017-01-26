/*
  * Generated by cppsrc.sh
  * On 2016-07-23 16:55:00,64
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ScriptComponent_H
#define ScriptComponent_H

#include <libSpace/src/Container/StaticVector.h>
#include <Core/Configuration.Core.h>

#include <Utils/LuaUtils.h>
#include "LuaUtils.h"

namespace MoonGlare::Core::Scripts::Component {

using namespace Core::Component;

class ScriptComponent
	: public AbstractComponent
	, public ComponentIDWrap<ComponentID::Script> {
public:
 	ScriptComponent(ComponentManager *Owner);
 	virtual ~ScriptComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) override;

	using LuaHandle = int;

	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_Active : 1; //Script has step function and it shall be called
			bool m_OnPerSecond : 1;//called when movedata.secondstep is true, need to be activated
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
		Handle m_SelfHandle;
		std::bitset<Configuration::Core::Events::MaxEventTypes> m_ExistingEventHandlers;

		void Reset() {
			m_Flags.m_Map.m_Valid = false;
		}
	};
	static_assert((sizeof(ScriptEntry) % 8) == 0, "Invalid ScriptEntry size!");
//	static_assert(std::is_pod<ScriptEntry>::value, "ScriptEntry must be pod!");

	bool GetObjectRootInstance(lua_State *lua, Entity Owner);//returns false on error; Owner shall be valid; returns OR GO on success and nothing on failure

	ScriptEntry* GetEntry(Handle h);
	ScriptEntry* GetEntry(Entity e) { return GetEntry(m_EntityMapper.GetHandle(e)); }

	template<typename EVENT>
	void HandleEventTemplate(const EVENT &ev) {
		auto *entry = GetEntry(ev.m_Source);
		if (!entry || !entry->m_Flags.m_Map.m_Valid || !entry->m_Flags.m_Map.m_Active) {
			return;
		}

		if (!entry->m_ExistingEventHandlers[EventInfo<EVENT>::GetClassID()]) {
			return;
		}

		auto lua = m_ScriptEngine->GetLua();
		LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
		Utils::Scripts::LuaStackOverflowAssert check(lua);
		//stack: -	

		int luatop = lua_gettop(lua);
		lua_pushcclosure(lua, Core::Scripts::LuaErrorHandler, 0);
		int errf = lua_gettop(lua);

		int index = entry - &m_Array[0] + 1;

		GetInstancesTable(lua);									//stack: self
		lua_rawgeti(lua, -1, index);							//stack: self Script/nil

		if (!lua_istable(lua, -1)) {
			lua_settop(lua, luatop);
			AddLogf(Error, "ScriptComponent: nil in lua script table at index: %d", index);
			return;
		}

		lua_getfield(lua, -1, EVENT::EventName);			//stack: self Script func/nil
		if (lua_isnil(lua, -1)) {
			AddLogf(Warning, "ScriptComponent: There is no %s function in component at index: %d", EVENT::EventName, index);
			lua_settop(lua, luatop);
			entry->m_ExistingEventHandlers[EventInfo<EVENT>::GetClassID()] = false;
			return;
		} else {
			//stack: self Script func
			lua_insert(lua, -2);							//stack: self func Script 
			luabridge::Stack<const EVENT*>::push(lua, &ev);  //stack: self func Script event

			if (!LuaSafeCall(lua, 2, 0, EVENT::EventName, errf)) {
				AddLogf(Error, "Failure during %s call for component #%lu", EVENT::EventName, index);
			}
		}
		lua_settop(lua, luatop);
	}
protected:
	Scripts::ScriptEngine *m_ScriptEngine;

	template<class T> using Array = Space::Container::StaticVector<T, Configuration::Storage::ComponentBuffer>;
	Array<ScriptEntry> m_Array;
	EntityMapper m_EntityMapper;

	void ReleaseComponent(lua_State *lua, size_t Index);

	void GetInstancesTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetInstancesTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}
	void GetGameObjectMetaTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetGameObjectMetaTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}
	void GetObjectRootTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetObjectRootTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}

	void *GetInstancesTableIndex() { return this; }
	void *GetGameObjectMetaTableIndex() { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 1); }
	void *GetObjectRootTableIndex() { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 2); }
private:
	bool InvalidateObjectRoot(lua_State *lua, Entity Owner);//returns false on error; Owner shall be not valid;

	bool InitGameObjectMetaTable(lua_State *lua);
//support functions
	int lua_GetScriptComponent(lua_State *lua, Entity Owner);
	int lua_GetComponentInfo(lua_State *lua, ComponentID cid, Entity Owner);
	int lua_MakeComponentInfo(lua_State *lua, ComponentID cid, Handle h, AbstractComponent *cptr);
	static int lua_DereferenceHandle(lua_State *lua);
	static int lua_SetComponentState(lua_State *lua);

//ScriptComponent api
	static int lua_DestroyComponent(lua_State *lua);
	static int lua_GetComponent(lua_State *lua);
	static int lua_SetPerSecond(lua_State *lua);
	static int lua_SetStep(lua_State *lua);
	static int lua_SetActive(lua_State *lua);

//GameObject api
	static int lua_CreateComponent(lua_State *lua);
	static int lua_SpawnChild(lua_State *lua);
	static int lua_DestroyObject(lua_State *lua);
	static int lua_Destroy(lua_State *lua);
	static int lua_SetName(lua_State *lua);
	static int lua_GetName(lua_State *lua);
	static int lua_FindChild(lua_State *lua);
	static int lua_GameObjectGetComponent(lua_State *lua);
	static int lua_GetParent(lua_State *lua);
	static int lua_GetFirstChild(lua_State *lua);
}; 

} //namespace MoonGlare::Core::Scripts::Component 

#endif
