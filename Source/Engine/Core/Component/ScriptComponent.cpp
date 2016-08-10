/*
  * Generated by cppsrc.sh
  * On 2016-07-23 16:55:00,64
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "AbstractComponent.h"
#include "ScriptComponent.h"
#include "ComponentManager.h"

namespace MoonGlare {
namespace Core {
namespace Component {

static_assert(sizeof(void*) == sizeof(Handle), "Size of void* must match the size of Handle!");

namespace lua {
	enum {
		SelfPtrUpValue = 1,
		HandleUpValue,
	};

	static const char *Function_Step = "Step";
	static const char *Function_OnCreate = "OnCreate";
	static const char *Function_OnDestroy = "OnDestroy";

	static const char *HandleMemberName = "Handle";
	static const char *ComponentIDMemberName = "ComponentId";
	static const char *DereferenceHandlerName = "Get";
	
	bool Lua_SafeCall(lua_State *lua, int args, int rets, const char *CaleeName) {
		try {
			AddLogf(ScriptCall, "Call to %s", CaleeName);
			lua_call(lua, args, rets);
			return true;
		}
		catch (::Core::Scripts::eLuaPanic &err) {
			AddLogf(Error, "Failure during call to %s message: %s", CaleeName, err.what());
			return false;
		}
	}
}

ScriptComponent::ScriptComponent(ComponentManager *Owner)
	: AbstractComponent(Owner) 
	, m_Allocated(0) {
}

ScriptComponent::~ScriptComponent() {
}

bool ScriptComponent::Initialize() {
	m_ScriptEngine = GetManager()->GetWorld()->GetScriptEngine();
	THROW_ASSERT(m_ScriptEngine, "No script engine instance!");

	Space::MemZero(m_Array);
	m_Allocated = 0;

	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);

	Utils::Scripts::LuaStackOverflowAssert check(lua);
	lua_pushlightuserdata(lua, (void *)this);
	lua_createtable(lua, m_Array.size(), 0);
	MoonGlare::Core::Scripts::PublishSelfLuaTable(lua, "ScriptComponent", this, -1);
	lua_settable(lua, LUA_REGISTRYINDEX);
	return true;
}

bool ScriptComponent::Finalize() {
	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
	MoonGlare::Core::Scripts::HideSelfLuaTable(lua, "ScriptComponent", this);
	lua_pushlightuserdata(lua, (void *)this);
	lua_pushnil(lua);
	lua_settable(lua, LUA_REGISTRYINDEX);
	return true;
}

void ScriptComponent::Step(const MoveConfig & conf) {
	if (m_Allocated == 0) {
		return;
	}

	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
	Utils::Scripts::LuaStackOverflowAssert check(lua);
	//stack: -
	lua_pushlightuserdata(lua, (void *)this);		//stack: selfptr
	lua_gettable(lua, LUA_REGISTRYINDEX);			//stack: self

	luabridge::Stack<const MoveConfig*>::push(lua, &conf); //stack: self movedata

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 0; i < m_Allocated; ++i) {
		auto &item = m_Array[i];
		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and ignore
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!GetHandleTable()->IsValid(this, item.m_Handle)) {
			AddLogf(Error, "ScriptComponent: invalid entity at index %d", i);
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!item.m_Flags.m_Map.m_StepFunction) {
			//silently ignore
			continue;
		}

		lua_pushinteger(lua, i + 1);		//stack: self movedata #component
		lua_gettable(lua, -3);				//stack: self movedata component/nil

		if (lua_isnil(lua, -1)) {
			lua_pop(lua, 1);
			AddLogf(Error, "ScriptComponent: nil in lua component table at index: %d", i);
			continue;
		}

		lua_getfield(lua, -1, lua::Function_Step); //stack: self movedata component Step/nil
		if (lua_isnil(lua, -1)) {
			lua_pop(lua, 2);
			item.m_Flags.m_Map.m_StepFunction = false;
			AddLogf(Warning, "ScriptComponent: There is no Step function in component at index: %d, disabling mapping", i);
			continue;
		}

		lua_insert(lua, -2); //stack: self movedata Step component 
		lua_pushvalue(lua, -3); //stack: self movedata Step component movedata

		if (!lua::Lua_SafeCall(lua, 2, 0, lua::Function_Step)) {
			AddLogf(Error, "Failure during OnStep call for component #%lu", i);
			item.m_Flags.m_Map.m_StepFunction = false;
		}

		//stack: self movedata Step 
	}

	lua_pop(lua, 2); //stack: -

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "ScriptComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseComponent(lua, LastInvalidEntry);
	}
}

void ScriptComponent::ReleaseComponent(lua_State *lua, size_t Index) {
	auto last = m_Allocated.load();
	if (!GetHandleTable()->SwapHandleIndexes(this, m_Array[Index].m_Handle, m_Array[last - 1].m_Handle)) {
		AddLogf(Error, "Failed to move last ScriptComponent entry to back!");
		return;
	}

	Utils::Scripts::LuaStackOverflowAssert check(lua);
	auto current = Index + 1; //lua index

	lua_pushlightuserdata(lua, (void *)this);
	lua_gettable(lua, LUA_REGISTRYINDEX);  //stack: self

	//set last in place of current and nil last
	lua_pushinteger(lua, current);		//stack: self current_id
	lua_gettable(lua, -2);				//stack: self current_table
	lua_pushinteger(lua, last);			//stack: self current_table last_id
	lua_gettable(lua, -3);				//stack: self current_table last_table
	lua_pushinteger(lua, current);      //stack: self current_table last_table current_id
	lua_insert(lua, -2);				//stack: self current_table current_id last_table 
	lua_settable(lua, -4);				//stack: self current_table 
	lua_pushinteger(lua, last);			//stack: self current_table last_id
	lua_pushnil(lua);					//stack: self current_table last_id nil
	lua_settable(lua, -4);				//stack: self current_table 
	//current remains on stack, so OnDestroy can be called

	auto &item = m_Array[Index];
	item.m_Flags.m_Map.m_Valid = false;

	std::swap(m_Array[Index], m_Array[last - 1]);

	lua_getfield(lua, -1, lua::Function_OnDestroy); //stack: self current_table OnDestroy/nil
	if (lua_isnil(lua, -1)) {
		lua_pop(lua, 1);
	} else {
		lua_pushvalue(lua, -2);			//stack: self current_table OnDestroy current_table
		if (!lua::Lua_SafeCall(lua, 1, 0, lua::Function_OnDestroy)) { //stack: self current_table
			//nothing there, nothing more to be logged
		}
	}
	//stack: self current_table
	lua_pop(lua, 2); //stack: -

	--m_Allocated;
}

bool ScriptComponent::Load(xml_node node, Entity Owner, Handle &hout) {
	auto name = node.child("Script").text().as_string(0);
	if (!name) {
		AddLogf(Error, "Attempt to load nameless script!");
		return false;
	}

	Handle &ch = hout;
	size_t index = m_Allocated++;

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();

	if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
		AddLogf(Error, "Failed to allocate handle!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
	Utils::Scripts::LuaStackOverflowAssert check(lua);

	if (!m_ScriptEngine->GetRegisteredScript(name)) {
		AddLogf(Error, "There is no such script: %s", name);
		GetHandleTable()->Release(this, ch);
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	entry.m_Owner = Owner;
	entry.m_Handle = ch;
	entry.m_Flags.SetAll();

	lua_createtable(lua, 0, 0);
	lua_insert(lua, -2);
	lua_setmetatable(lua, -2);

	luabridge::Stack<Entity*>::push(lua, &Owner);
	lua_setfield(lua, -2, "Entity");
	lua_pushlightuserdata(lua, this);
	lua_setfield(lua, -2, "Component");
	lua_pushlightuserdata(lua, ch.GetVoidPtr());
	lua_setfield(lua, -2, lua::HandleMemberName);

	lua_pushlightuserdata(lua, this);
	lua_pushlightuserdata(lua, ch.GetVoidPtr());
	lua_pushcclosure(lua, &lua_DestroyComponent, 2);
	lua_setfield(lua, -2, "DestroyComponent");

	lua_pushlightuserdata(lua, this);
	lua_pushlightuserdata(lua, ch.GetVoidPtr());
	lua_pushcclosure(lua, &lua_GetComponent, 2);
	lua_setfield(lua, -2, "GetComponent");
	
	//TODO: DestroyObject(void/other)

	lua_getfield(lua, -1, lua::Function_OnCreate);
	if (lua_isnil(lua, -1)) {
		lua_pop(lua, 1);
		m_Array[index].m_Flags.m_Map.m_OnCreateFunction = false;
	} else {
		lua_pushvalue(lua, -2);
		if (!lua::Lua_SafeCall(lua, 1, 0, lua::Function_OnCreate)) {
			//no need for more logging
			m_Array[index].m_Flags.m_Map.m_OnCreateFunction = false;
		}
	}

	lua_pushlightuserdata(lua, (void *)this);
	lua_gettable(lua, LUA_REGISTRYINDEX);
	lua_insert(lua, -2);

	//lua index starts from 1
	lua_pushinteger(lua, static_cast<int>(index) + 1);
	lua_insert(lua, -2);

	lua_settable(lua, -3);
	lua_pop(lua, 1);

	return true;
}

bool ScriptComponent::GetInstanceHandle(Entity Owner, Handle &hout) {
	LOG_NOT_IMPLEMENTED();
	return false;
	//TODO
}

//-------------------------------------------------------------------------------------------------

bool ExtractHandleFromArgument(lua_State *lua, int location, Handle &h, bool AllowSelfHandle = true) {
	int type = lua_type(lua, location);
	switch (type) {
	case LUA_TNIL:
		if (!AllowSelfHandle) {//TODO
			throw "!AllowSelfHandle";
		}
		h = Handle::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::HandleUpValue)));
		break;
	case LUA_TLIGHTUSERDATA:
		h = Handle::FromVoidPtr(lua_touserdata(lua, location));
		break;
	case LUA_TTABLE:
		lua_getfield(lua, location, lua::HandleMemberName);
		if (!lua_islightuserdata(lua, -1)) {
			lua_pop(lua, 1);
			return false;
		}
		h = Handle::FromVoidPtr(lua_touserdata(lua, -1));
		lua_pop(lua, 1);
		break;
	default:
		return false;
	}
	return true;
}

int ScriptComponent::lua_DestroyComponent(lua_State *lua) {
	Handle h;
	if (!ExtractHandleFromArgument(lua, -1, h)) {
		AddLogf(Error, "ScriptComponent::DestroyComponent: Error: Invalid argument #1: unknown type!");
		lua_pushboolean(lua, 0);
		return 1;
	}

	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	size_t index;
	if (!This->GetHandleTable()->GetHandleIndex(This, h, index)) {
		AddLogf(Error, "ScriptComponent::DestroyComponent: Error: Invalid argument #1: invalid handle");
		lua_pushboolean(lua, 0);
		return 1;
	}

	This->m_Array[index].m_Flags.m_Map.m_Valid = false;

	lua_pushboolean(lua, 1);
	return 1;
}

int ScriptComponent::lua_DestroyObject(lua_State *lua) {
	Handle h;
	if (!ExtractHandleFromArgument(lua, -1, h)) {
		AddLogf(Error, "ScriptComponent::DestroyObject: Error: Invalid argument #1: unknown type!");
		lua_pushboolean(lua, 0);
		return 1;
	}

	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	size_t index;
	if (!This->GetHandleTable()->GetHandleIndex(This, h, index)) {
		AddLogf(Error, "ScriptComponent::DestroyObject: Error: Invalid argument #1: invalid handle");
		lua_pushboolean(lua, 0);
		return 1;
	}

	//auto &item = This->m_Array[index];
	//auto *scene = static_cast<GameScene*>(This->GetManager()->GetScene());
	//auto reg = scene->GetObjectRegister();

	LOG_NOT_IMPLEMENTED();
	//TODO: lua api DestroyObject
	AddLog(Error, "DestroyObject is not yet supported");

	lua_pushboolean(lua, 0);
	return 1;
}

int ScriptComponent::lua_GetComponent(lua_State *lua) {
	int argc = lua_gettop(lua);

	ComponentID cid = 0;
	Handle RequestHandle;

	switch (argc) {
	case 2:
		if (!lua_isnumber(lua, -1)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #1: invalid type! (single argument mode)");
			return 0;
		}
		cid = static_cast<ComponentID>(lua_tointeger(lua, -1));
		if (!ExtractHandleFromArgument(lua, -2, RequestHandle)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #2: unknown type!");
			return 0;
		}
		break;
	case 3:
		if (!lua_isnumber(lua, -2)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #1: invalid type! (double argument mode)");
			return 0;
		}
		if (!lua_islightuserdata(lua, -1)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #32: invalid type! (double argument mode)");
			return 0;
		}
		cid = static_cast<ComponentID>(lua_tointeger(lua, -2));
		RequestHandle = Handle::FromVoidPtr(lua_touserdata(lua, -1));
		break;
	default:
		AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument count");
		return 0;
	}

	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	auto cptr = This->GetManager()->GetComponent(cid);
	if (!cptr) {
		AddLogf(Error, "ScripComponent::GetComponent: Error: There is no component %d", cid);
		return 0;
	}

	Entity OwnerEntity;
	auto HandleTable = This->GetManager()->GetWorld()->GetHandleTable();
	if (!HandleTable->GetHandleParentEntity(This, RequestHandle, OwnerEntity)) {
		AddLogf(Error, "ScripComponent::GetComponent: Error: Invalid owner handle!");
		return 0;
	}

	Handle ComponentHandle;

	if (!cptr->GetInstanceHandle(OwnerEntity, ComponentHandle)) {
		AddLogf(Error, "ScripComponent::GetComponent: no component instance for requested object");
		return 0;
	}

	lua_createtable(lua, 0, 5);

	lua_pushlightuserdata(lua, ComponentHandle.GetVoidPtr());
	lua_setfield(lua, -2, lua::HandleMemberName);

	lua_pushinteger(lua, static_cast<int>(cid));
	lua_setfield(lua, -2, lua::ComponentIDMemberName);

	lua_pushlightuserdata(lua, cptr);
	lua_pushlightuserdata(lua, ComponentHandle.GetVoidPtr());
	lua_pushcclosure(lua, &lua_DereferenceHandle, 2);
	lua_setfield(lua, -2, lua::DereferenceHandlerName);

	return 1;
}

int ScriptComponent::lua_DereferenceHandle(lua_State *lua) {
	void *voidcptr = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	AbstractComponent *cptr = reinterpret_cast<AbstractComponent*>(voidcptr);

	Handle h = Handle::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::HandleUpValue)));

	int rets = 0;
	if (!cptr->PushEntryToLua(h, lua, rets)) {
		AddLogf(Error, "ComponentInstanceInfo::Get: Error: Component '%s' does not support lua api", typeid(*cptr).name());
		return 0;
	}

	return rets;
}

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 
