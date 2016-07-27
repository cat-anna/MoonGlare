/*
  * Generated by cppsrc.sh
  * On 2015-03-24 17:28:03,29
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "GlobalContext.h"

namespace Core {
namespace Scripts {

class ContextBaseElement;
using ContextElementPtr = std::unique_ptr < ContextBaseElement > ;

class ContextBaseElement : public cRootClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(ContextBaseElement, cRootClass);
	DISABLE_COPY();
public:
	ContextBaseElement(ContextBaseElement *Parent, const string& Name, int Type) : 
			BaseClass(),
			m_Parent(Parent),
			m_Name(Name),
			m_Type (Type){ 
	}

	virtual int Push(lua_State *lua) = 0;
	/** source element is on top of the lua stack (index -1) */
	virtual int Set(lua_State *lua, int index) = 0;

	virtual bool LoadMeta(const xml_node node) = 0;
	virtual bool SaveMeta(xml_node node) const = 0;

	virtual void Dump(std::ostream& o) const = 0;

	const string& GetName() const { return m_Name; }
	ContextBaseElement* GetParent() const { return m_Parent; }
	int GetType() const { return m_Type; }
	bool CanBeReset(int othertype) const { return m_Type != LUA_TTABLE && othertype == m_Type; }

	string FullName() const {
		std::stack <const ContextBaseElement*> s;
		auto *pu = this;
		while (pu) {
			s.push(pu);
			pu = pu->GetParent();
		}
		string st;
		st.reserve(256);
		while (!s.empty()) {
			auto px = s.top();
			s.pop();
			st += px->GetName();
			if (px != this)
				st += '.';
		}
		return std::move(st);
	}
protected:
	ContextBaseElement *m_Parent;

	static int ErrorLuaFunc(lua_State *lua) {
		AddLog(Error, "Attempt to call nonexisting function!");
		//lua_pushstring(lua, "Call to disabled function!");
		//lua_error(lua);
		return 0;
	}

	template<class T>
	void GetMetaTable(lua_State *lua) {
		luabridge::lua_rawgetp (lua, LUA_REGISTRYINDEX, luabridge::ClassInfo<T>::getClassKey());
		bool ch = lua_istable (lua, -1);
		if(!ch)
			throw std::runtime_error("An attempt to push nonexistent context group type!");//LuaBrigeException
	}

	template<class T>
	void SetMetaTable(lua_State *lua) {
		GetMetaTable<T>(lua);
		lua_setmetatable (lua, -2);
	}
private:
	int m_Type;
	string m_Name;
};

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(ContextBaseElement);

//---------------------------------------------------------------------------------------

struct LuaTypeEnumConverter : Space::EnumConverter < int, LUA_TNIL > {
	LuaTypeEnumConverter() {
		Add("Nil", LUA_TNIL);
		Add("Bool", LUA_TBOOLEAN);
#define LUA_TLIGHTUSERDATA	2
		Add("Float", LUA_TNUMBER);
		Add("String", LUA_TSTRING);
		Add("Group", LUA_TTABLE);
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8
	}
};
using LuaTypeEnum = Space::EnumConverterHolder < LuaTypeEnumConverter > ;

struct ElementConstructionTable {
	using func_t = ContextElementPtr(*)(ContextBaseElement *Parent, const string& Name);

	enum {
		TableSize = 10,
	};

	ElementConstructionTable();

	func_t operator[](int index) const { return m_table[index]; }
protected:
	func_t m_table[TableSize];
	static ContextElementPtr EmptyFunction(ContextBaseElement *Parent, const string& Name) { return nullptr; }
};

const ElementConstructionTable ElementConstructionTableInstance;

//---------------------------------------------------------------------------------------

class ContextStringElement : public ContextBaseElement {
	SPACERTTI_DECLARE_STATIC_CLASS(ContextStringElement, ContextBaseElement);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ContextStringElement(ContextBaseElement *Parent, const string& Name):
		BaseClass(Parent, Name, LUA_TSTRING), m_Value() { }

	virtual bool LoadMeta(const xml_node node) override {
		auto val = node.text().as_string(0);
		if (!val)
			return false;
		m_Value = val;
		return true;
	}
	virtual bool SaveMeta(xml_node node) const override {
		node.text() = m_Value.c_str();
		return true;
	}

	virtual int Push(lua_State *lua) override {
		LOCK_MUTEX(m_mutex);
		if (m_Value.empty())
			lua_pushstring(lua, "");
		else
			lua_pushlstring(lua, m_Value.c_str(), m_Value.length());
		AddLogf(Debug, "Pushed value '%s' which is '%s'", FullName().c_str(), m_Value.c_str());
		return 1;
	}
	virtual int Set(lua_State *lua, int index) override {
		LOCK_MUTEX(m_mutex);
		const char *c = lua_tostring(lua, index);
		if (!c) {
			AddLog(Warning, "Lua was unable to convert type " << lua_type(lua, index) << " to string!");
			m_Value = "";
		} else
			m_Value = c;
		AddLogf(Debug, "Value '%s' set to '%s'", FullName().c_str(), m_Value.c_str());
		return 0;
	}
	void Set(string value) { m_Value.swap(value); }

	virtual void Dump(std::ostream& o) const override { 
		o << FullName() << " = '" << m_Value << "'\n";
	}
	
	static void Register(lua_State *lua) { }
protected:
	string m_Value;
	std::recursive_mutex m_mutex;
};

SPACERTTI_IMPLEMENT_STATIC_CLASS(ContextStringElement);

//---------------------------------------------------------------------------------------

class ContextNumberElement : public ContextBaseElement {
	SPACERTTI_DECLARE_STATIC_CLASS(ContextNumberElement, ContextBaseElement);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ContextNumberElement(ContextBaseElement *Parent, const string& Name):
		BaseClass(Parent, Name, LUA_TNUMBER), m_Value(0) { }

	virtual bool LoadMeta(const xml_node node) override {
		auto val = node.text().as_string(0);
		if (!val)
			return false;
		m_Value = node.text().as_double();
		return true;
	}
	virtual bool SaveMeta(xml_node node) const override {
		node.text() = m_Value;
		return true;
	}

	virtual int Push(lua_State *lua) override {
		lua_pushnumber(lua, m_Value);

	//	lua_newtable(lua);

	//	lua_pushstring(lua, "__newindex");
	//	lua_pushcfunction(lua, &ErrorLuaFunc);
	//	lua_settable(lua, -3);
	//	lua_pushstring(lua, "__index");
	//	lua_pushcfunction(lua, &Index);
	//	lua_settable(lua, -3);
	////hide metatable
	//	lua_pushstring(lua, "__metatable");
	//	lua_pushnil(lua);
	//	lua_settable(lua, -3);
	////write keyvalue
	//	lua_pushstring(lua, "ptr");
	//	lua_pushlightuserdata(lua, (void*)this);
	//	lua_settable(lua, -3);

	//	lua_setmetatable (lua, -2);(with metatable)

		AddLogf(Debug, "Pushed value '%s' which is %f", FullName().c_str(), m_Value);
		return 1;
	}
	virtual int Set(lua_State *lua, int index) override{
		m_Value = lua_tonumber(lua, index);
		AddLogf(Debug, "Value '%s' set to %f", FullName().c_str(), m_Value);
		return 0;
	}
	void Set(lua_Number value) { m_Value = value; }

	virtual void Dump(std::ostream& o) const override { 
		o << FullName() << " = " << m_Value << "\n";
	}

	static void Register(lua_State *lua) {
#if 0
		auto *keyvalue = luabridge::ClassInfo<ThisClass>::getClassKey();
		lua_newtable(lua);

		lua_pushstring(lua, "__newindex");
		lua_pushcfunction(lua, &ErrorLuaFunc);
		lua_settable(lua, -3);

		lua_pushstring(lua, "__index");
		lua_pushcfunction(lua, &Index);
		lua_settable(lua, -3);

	//hide metatable
		lua_pushstring(lua, "__metatable");
		lua_pushnil(lua);
		lua_settable(lua, -3);
	//write keyvalue
		lua_pushstring(lua, "keyvalue");
		lua_pushlightuserdata(lua, (void*)keyvalue);
		lua_settable(lua, -3);

		lua_pushstring(lua, "Add");
		lua_pushcfunction(lua, &NumberAdd);
		lua_settable(lua, -3);

		//luabridge::ClassInfo<ThisClass>::getStaticKey();
		luabridge::lua_rawsetp(lua, LUA_REGISTRYINDEX, keyvalue);
#endif
	}
protected:
	lua_Number m_Value;

	static ThisClass* NumberGetSelf(lua_State *lua, int index) {
		if (!lua_isnumber(lua, index)) {
			AddLog(Error, "Invalid self parameter!");
			return nullptr;
		}
		lua_getmetatable(lua, index);
		if (!lua_istable(lua, -1)) {
			AddLog(Error, "Invalid number metatable");
			return nullptr;
		}
		lua_pushstring(lua, "ptr");
		lua_gettable(lua, -2);
		cRootClass *root = (cRootClass*)lua_touserdata(lua, -1);
		auto *ptr = dynamic_cast<ThisClass*>(root);
		if (!ptr) {
			AddLog(Error, "Self pointer has invalid type!");
			return nullptr;
		}
		lua_pop(lua, 2);
		return ptr;
	}

	static int NumberAdd(lua_State *lua) {
		auto *ptr = NumberGetSelf(lua, -2);
		if (!ptr) return 0;
		auto param = lua_tonumber(lua, -1);
		auto &val = ptr->m_Value;
		val += param;
		lua_pushnumber(lua, val);
		AddLogf(Debug, "Atomic add %f into '%s' new value '%s'", param, ptr->FullName().c_str(), val);
		return 1;
	}
	static int NumberSub(lua_State *lua) {
		auto *ptr = NumberGetSelf(lua, -2);
		if (!ptr) return 0;
		auto param = lua_tonumber(lua, -1);
		auto &val = ptr->m_Value;
		val -= param;
		lua_pushnumber(lua, val);
		AddLogf(Debug, "Atomic sub %f into '%s' new value '%s'", param, ptr->FullName().c_str(), val);
		return 1;
	}
	static int NumberMul(lua_State *lua) {
		auto *ptr = NumberGetSelf(lua, -2);
		if (!ptr) return 0;
		auto param = lua_tonumber(lua, -1);
		auto &val = ptr->m_Value;
		val *= param;
		lua_pushnumber(lua, val);
		AddLogf(Debug, "Atomic mul %f into '%s' new value '%s'", param, ptr->FullName().c_str(), val);
		return 1;
	}
	static int NumberDiv(lua_State *lua) {
		auto *ptr = NumberGetSelf(lua, -2);
		if (!ptr) return 0;
		auto param = lua_tonumber(lua, -1);
		auto &val = ptr->m_Value;
		val /= param;
		lua_pushnumber(lua, val);
		AddLogf(Debug, "Atomic div %f into '%s' new value '%s'", param, ptr->FullName().c_str(), val);
		return 1;
	}
	static int NumberSet(lua_State *lua) {
		auto *ptr = NumberGetSelf(lua, -2);
		if (!ptr) return 0;
		auto param = lua_tonumber(lua, -1);
		auto &val = ptr->m_Value;
		val = param;
		lua_pushnumber(lua, val);
		AddLogf(Debug, "Atomic set '%s' new value '%s'", param, ptr->FullName().c_str());
		return 1;
	}

	using ContextNumberFunction = int(*)(lua_State *lua);
	struct ContextNumberFunctionMap : public std::unordered_map < string, ContextNumberFunction > {
		ContextNumberFunctionMap() {
			insert(std::make_pair("add", &ThisClass::NumberAdd));
			insert(std::make_pair("sub", &ThisClass::NumberSub));
			insert(std::make_pair("div", &ThisClass::NumberDiv));
			insert(std::make_pair("mul", &ThisClass::NumberMul));
			insert(std::make_pair("set", &ThisClass::NumberSet));
		}
	};

	static const ContextNumberFunctionMap _ContextNumberFunctionMap;

	static int Index(lua_State *lua) {
		auto *ptr = NumberGetSelf(lua, -2);
		if (!ptr) return 0;

		const char* Key = lua_tostring(lua, -1);
		if (!Key) {
			AddLog(Error, "Invalid key value!");
			return 0;
		}

		auto it = _ContextNumberFunctionMap.find(Key);
		if (it == _ContextNumberFunctionMap.end())
			return 0;

		lua_pushcfunction(lua, it->second);
		return 1;
	}
};

SPACERTTI_IMPLEMENT_STATIC_CLASS(ContextNumberElement);
const ContextNumberElement::ContextNumberFunctionMap ContextNumberElement::_ContextNumberFunctionMap;

//---------------------------------------------------------------------------------------

const char* BooleanTable[] ={ "False", "True" };

class ContextBooleanElement : public ContextBaseElement {
	SPACERTTI_DECLARE_STATIC_CLASS(ContextBooleanElement, ContextBaseElement);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ContextBooleanElement(ContextBaseElement *Parent, const string& Name):
		BaseClass(Parent, Name, LUA_TBOOLEAN), m_Value(false) { }

	virtual bool LoadMeta(const xml_node node) override {
		auto val = node.text().as_string(0);
		if (!val)
			return false;
		m_Value = node.text().as_bool();
		return true;
	}
	virtual bool SaveMeta(xml_node node) const override {
		node.text() = BooleanTable[m_Value?1:0];
		return true;
	}

	virtual int Push(lua_State *lua) override {
		lua_pushboolean(lua, m_Value);

		AddLogf(Debug, "Pushed value '%s' which is %s", FullName().c_str(), BooleanTable[m_Value?1:0]);
		return 1;
	}
	virtual int Set(lua_State *lua, int index) override{
		m_Value = lua_toboolean(lua, index);
		AddLogf(Debug, "Value '%s' set to %s", FullName().c_str(), BooleanTable[m_Value?1:0]);
		return 0;
	}

	virtual void Dump(std::ostream& o) const override { 
		o << FullName() << " = " << BooleanTable[m_Value?1:0] << "\n";
	}

	static void Register(lua_State *lua) { }
protected:
	bool m_Value;
};

SPACERTTI_IMPLEMENT_STATIC_CLASS(ContextBooleanElement);
//---------------------------------------------------------------------------------------

class ContextNilElement : public ContextBaseElement {
	SPACERTTI_DECLARE_STATIC_CLASS(ContextNilElement, ContextBaseElement);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ContextNilElement(ContextBaseElement *Parent, const string& Name):
		BaseClass(Parent, Name, LUA_TNIL) { }

	virtual bool LoadMeta(const xml_node node) override {
		return true;
	}
	virtual bool SaveMeta(xml_node node) const override {
		return true;
	}

	virtual int Push(lua_State *lua) override {
		lua_pushnil(lua);
		AddLogf(Debug, "Pushed value '%s' which is nil", FullName().c_str());
		return 1;
	}
	virtual int Set(lua_State *lua, int index) override {
		AddLogf(Debug, "Nil value '%s' cannot be set!", FullName().c_str());
		return 0;
	}

	virtual void Dump(std::ostream& o) const override { 
		o << FullName() << " = nil\n";
	}

	static void Register(lua_State *lua) { }
protected:
};

SPACERTTI_IMPLEMENT_STATIC_CLASS(ContextNilElement);

//---------------------------------------------------------------------------------------

template <class T>
ContextElementPtr DynamicElementConstructorFunc(ContextBaseElement *Parent, const string& Name) {
	return std::make_unique<T>(Parent, Name);
}

ElementConstructionTable::ElementConstructionTable() {
	for (int i = 0; i < TableSize; ++i)
		m_table[i] = &EmptyFunction;

	//m_table[LUA_TNIL] = &DynamicElementConstructorFunc < ContextNilElement > ;
	m_table[LUA_TBOOLEAN] = &DynamicElementConstructorFunc < ContextBooleanElement > ;
#define LUA_TLIGHTUSERDATA	2
	m_table[LUA_TNUMBER] = &DynamicElementConstructorFunc < ContextNumberElement > ;
	m_table[LUA_TSTRING] = &DynamicElementConstructorFunc < ContextStringElement > ;
	m_table[LUA_TTABLE] = &DynamicElementConstructorFunc < ContextGroup > ;
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8
}

//---------------------------------------------------------------------------------------

class ContextGroup : public ContextBaseElement {
	SPACERTTI_DECLARE_STATIC_CLASS(ContextGroup, ContextBaseElement);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ContextGroup(ContextBaseElement *Parent, const string& Name):
		BaseClass(Parent, Name, LUA_TTABLE) { }

	virtual bool LoadMeta(const xml_node node) override {
		XML::ForEachChild(node, "Item", [this](const xml_node node) {
			auto ch_type = node.attribute("Type").as_string(0);
			auto ch_name = node.attribute("Name").as_string(0);
			if (!ch_name || !ch_name) {
				AddLog(Warning, "Invalid context item definition!");
				return 0;
			}
			auto type = LuaTypeEnum::ConvertSafe(ch_type);
			string Key(ch_name);
			auto element = ElementConstructionTableInstance[type](this, Key);
			if (!element) {
				AddLog(Warning, "Unable to create context item!");
				return 0;
			}
			if (!element->LoadMeta(node)) {
				AddLog(Warning, "Unable to load context item!");
				return 0;
			}
			m_Map[Key].swap(element);
			return 0;
		});
		return true;
	}
	virtual bool SaveMeta(xml_node node) const override {
		for (auto &it: m_Map) {
			xml_node item = node.append_child("Item");
			auto *val = it.second.get();
			item.append_attribute("Name") = it.first.c_str();
			item.append_attribute("Type") = LuaTypeEnum::ToString(val->GetType()).c_str();
			if (!val->SaveMeta(item)) {
				AddLog(Error, "Unable to save context item " << val->FullName());
			}
		}
		return true;
	}

	virtual int Push(lua_State *lua) override {
		LOCK_MUTEX(m_Mutex);
		lua_pushlightuserdata(lua, this);
		SetMetaTable<ThisClass>(lua);
		AddLogf(Debug, "Pushed table '%s'", FullName().c_str());
		return 1;
	}
	virtual int Set(lua_State *lua, int index) override {
		if (!lua_istable(lua, index)) {
			AddLog(Error, "Unable to set global table element to non table value!");
			return 0;
		}
		LOCK_MUTEX(m_Mutex);
		lua_pushnil(lua);
		AddLogf(Debug, "Setting table element '%s'", FullName().c_str());

		while (lua_next(lua, index - 1) != 0) {
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			//printf("%s - %s\n",
				//lua_typename(lua, lua_type(lua, -2)),
				//lua_typename(lua, lua_type(lua, -1)));
			/* removes 'value'; keeps 'key' for next iteration */
			char keybuffer[128];
			const char *ckey;

			int ktype = lua_type(lua, -2);
			switch (ktype) {
			case LUA_TSTRING:
				ckey = lua_tostring(lua, -2);
				break;
			case LUA_TNUMBER:
				sprintf(keybuffer, "%d", lua_tointeger(lua, -2));
				ckey = keybuffer;
				break;
			default:
				AddLog(Error, "Table key not convertible to string!");
				lua_pop(lua, 1);
				continue;
			}
			
			SetElement(lua, ckey, -1);
			lua_pop(lua, 1);
		}
		AddLogf(Debug, "Setting table element '%s' is done", FullName().c_str());
		return 0;
	}

	static void Register(lua_State *lua) {
		auto *keyvalue = luabridge::ClassInfo<ThisClass>::getClassKey();
		lua_newtable(lua);

		lua_pushstring(lua, "__newindex");
		lua_pushcfunction(lua, &NewIndex);
		lua_settable(lua, -3);

		lua_pushstring(lua, "__index");
		lua_pushcfunction(lua, &Index);
		lua_settable(lua, -3);

	//hide metatable
		lua_pushstring(lua, "__metatable");
		lua_pushnil(lua);
		lua_settable(lua, -3);
	//write keyvalue
		lua_pushstring(lua, "keyvalue");
		lua_pushlightuserdata(lua, (void*)keyvalue);
		lua_settable(lua, -3);

		//luabridge::ClassInfo<ThisClass>::getStaticKey();
		luabridge::lua_rawsetp (lua, LUA_REGISTRYINDEX, keyvalue);
	}

	virtual void Dump(std::ostream& o) const override {
		if (m_Map.empty()) {
			o << FullName() << " = { }\n";
			return;
		}
		for (auto &it : m_Map)
			it.second->Dump(o);
	}

	using GroupMap = std::unordered_map < string, ContextElementPtr > ;
protected:
	GroupMap m_Map;
	std::recursive_mutex m_Mutex;

	inline int SetElement(const string &Key, const string& value) {
		LOCK_MUTEX(m_Mutex);

		auto it = m_Map.find(Key);
		bool KeyExists = it != m_Map.end();

		auto element = new ContextStringElement(this, Key);
		element->Set(value);
		if (KeyExists)
			it->second.reset(element);
		else
			m_Map[Key].reset(element);

		return 0;
	}
	inline int SetElement(const string &Key, lua_Number value) {
		LOCK_MUTEX(m_Mutex);

		auto it = m_Map.find(Key);
		bool KeyExists = it != m_Map.end();

		auto element = new ContextNumberElement(this, Key);
		element->Set(value);
		if (KeyExists)
			it->second.reset(element);
		else
			m_Map[Key].reset(element);

		return 0;
	}

	inline int SetElement(lua_State *lua, const string &Key, int index) {
		LOCK_MUTEX(m_Mutex);
		int type = lua_type(lua, index);
		if (type < 0) type = LUA_TNIL;

		auto it = m_Map.find(Key);
		bool KeyExists = it != m_Map.end();

		if (type == LUA_TNIL) {
			if (KeyExists)
				m_Map.erase(it);
			return 0;
		}

		if (!KeyExists || !it->second->CanBeReset(type)) {
			ContextElementPtr element;
			element = ElementConstructionTableInstance[type](this, Key);
			if (!element) {
				AddLog(Error, "Unable to create context element for lua type " << type);
				return 0;
			}
			element->Set(lua, index);
			if (KeyExists)
				it->second.swap(element);
			else
				m_Map[Key].swap(element);
		} else {
			it->second->Set(lua, index);
		}
		return 0;
	}

	static int Index(lua_State *lua) {
		if (!lua_islightuserdata(lua, -2)) {
			AddLog(Error, "Invalid self pointer in index method!");
			return 0;
		}
		cRootClass *root = (cRootClass*)lua_touserdata(lua, -2);
		auto *ptr = dynamic_cast<ThisClass*>(root);
		if (!ptr) {
			AddLog(Error, "Self pointer has invalid type!");
			return 0;
		}
		const char* Key = lua_tostring(lua, -1);
		if (!Key) {
			AddLog(Error, "Invalid key type!");
			return 0;
		}
		LOCK_MUTEX(ptr->m_Mutex);
		auto it = ptr->m_Map.find(Key);
		if (it == ptr->m_Map.end() || !it->second) {
			lua_pushnil(lua);
			return 1;
		}
		//if (!it->second) {
		//	ContextElementPtr element;
		//	element = ElementConstructionTableInstance[LUA_TTABLE](ptr, Key);
		//	if (!element) {
		//		AddLog(Error, "Unable to create context element for implicit table!");
		//		lua_pushnil(lua);
		//		return 1;
		//	} else {
		//		auto *eptr = element.get();
		//		ptr->m_Map[Key].swap(element);
		//		return eptr->Push(lua);
		//	}
		//	return 1;
		//}
		return it->second->Push(lua);
	}

	static int NewIndex(lua_State *lua) {
		if (!lua_islightuserdata(lua, -3)) {
			AddLog(Error, "Invalid self pointer in index method!");
			return 0;
		}
		cRootClass *root = (cRootClass*)lua_touserdata(lua, -3);
		auto *ptr = dynamic_cast<ThisClass*>(root);
		if (!ptr) {
			AddLog(Error, "Self pointer has invalid type!");
			return 0;
		}
		if (lua_type(lua, -2) == LUA_TNIL) {
			char idx[16];
			sprintf(idx, "%d", ptr->m_Map.size() + 1);
			//ptr->SetElement("Count", ptr->m_Map.size() + 1);
			return ptr->SetElement(lua, idx, -1);
		}

		const char* Key = lua_tostring(lua, -2);
		if (!Key) {
			AddLog(Error, "Invalid key type!");
			return 0;
		}
		return ptr->SetElement(lua, Key, -1);
	}
};

SPACERTTI_IMPLEMENT_STATIC_CLASS(ContextGroup);

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(GlobalContext);
RegisterApiDerivedClass(GlobalContext, &GlobalContext::RegisterScriptApi);
RegisterApiInstance(GlobalContext, &GlobalContext::Instance, "GlobalContext");

GlobalContext::GlobalContext():
		BaseClass() {
	SetThisAsInstance();
}

GlobalContext::~GlobalContext() {
}

//---------------------------------------------------------------------------------------

void GlobalContext::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cContext")
#ifdef DEBUG_DUMP
		.addFunction("Dump", &ThisClass::LogDump)
#endif
	.endClass()
	
	//.deriveClass<ContextBaseElement, ContextBaseElement::BaseClass>("cContextElement")
	//.endClass()

	//.deriveClass<ContextGroup, ContextGroup::BaseClass>("cContextGroup")
	//.endClass()

	;
}

//---------------------------------------------------------------------------------------

bool GlobalContext::Install(lua_State *lua) {
	try {
		ContextGroup::Register(lua);
		ContextNumberElement::Register(lua);
		ContextStringElement::Register(lua);
		ContextBooleanElement::Register(lua);

		//auto luabridge::getGlobalNamespace(lua)
		//	.addProperty<ContextGroup*, ContextGroup*>("global", &ThisClass::GetGlobalContext, nullptr)
		//	.addProperty<ContextGroup*, ContextGroup*>("permanent", &ThisClass::GetPermanentContext, nullptr)
		//	;

		m_PermanentBase->Push(lua);
		lua_setglobal(lua, "static");
	}
	catch(...) {
		AddLog(Error, "An error has occur during global context installation");
		throw;
	}
	AddLog(Hint, "Global context installed!");
	return true;
}

//---------------------------------------------------------------------------------------

void GlobalContext::CleanContext() {
	REQUIRE_REIMPLEMENT();
}

bool GlobalContext::Initialize() {
//	m_GlobalBase.reset(new ContextGroup(nullptr, "global"));
	m_PermanentBase.reset(new ContextGroup(nullptr, "static"));

	xml_document doc;
	doc.load_file("Static.xml");
	m_PermanentBase->LoadMeta(doc.document_element());

	return true;
}

bool GlobalContext::Finalize() {
	xml_document doc;
	m_PermanentBase->SaveMeta(doc.append_child("Static"));
	doc.save_file("Static.xml");

//	m_GlobalBase.reset();
	m_PermanentBase.reset();
	return true;
}

//---------------------------------------------------------------------------------------

#ifdef DEBUG_DUMP
void GlobalContext::Dump(std::ostream& out) {
	out << "Global context dump:\n";

	
	auto ret = ::MoonGlare::Core::GetScriptEngine()->RunFunction<const char *>("Dump_GlobalCtx");
	if (ret) {
		out << "GlobalBase:\n";
		out << ret;
		out << "\n";
	} else {
		out << "Failed to obtain global ctx!\n";
	}

	if (m_PermanentBase) {
		out << "PermanentBase:\n";
		m_PermanentBase->Dump(out);
		out << "\n";
	}
}

void GlobalContext::LogDump() {
	std::stringstream ss;
	ss << "\n";
	Dump(ss);
	string s = ss.str();
	AddLog(Debug, s);
}
#endif

} //namespace Scripts 
} //namespace Core 
