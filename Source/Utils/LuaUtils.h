#pragma once

namespace Utils {
namespace Scripts {

#if 0
inline void stackDump(lua_State *L) {
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(L, i);
		printf("%s", lua_typename(L, t));
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */
}
#endif

//----------------------------------------------------

template<class T> inline T Lua_to(lua_State *lua, int idx);
template<> inline bool Lua_to<bool>(lua_State *lua, int idx) { return lua_toboolean(lua, idx) != 0; }
template<> inline float Lua_to<float>(lua_State *lua, int idx) { return static_cast<float>(lua_tonumber(lua, idx)); }
template<> inline double Lua_to<double>(lua_State *lua, int idx) { return static_cast<double>(lua_tonumber(lua, idx)); }
template<> inline const char* Lua_to<const char*>(lua_State *lua, int idx) { return lua_tostring(lua, idx); }
template<> inline int Lua_to<int>(lua_State *lua, int idx) { return lua_tointeger(lua, idx); }

template<class T> inline void Lua_push(lua_State *lua, T t);
template<> inline void Lua_push<bool>(lua_State *lua, bool t) { lua_pushboolean(lua, t); }
template<> inline void Lua_push<float>(lua_State *lua, float t) { lua_pushnumber(lua, static_cast<LUA_NUMBER>(t)); }
template<> inline void Lua_push<double>(lua_State *lua, double t) { lua_pushnumber(lua, static_cast<LUA_NUMBER>(t)); }
template<> inline void Lua_push<const char*>(lua_State *lua, const char* t) { lua_pushstring(lua, t); }
template<> inline void Lua_push<int>(lua_State *lua, int t) { lua_pushinteger(lua, t); }

template<class T> bool inline Lua_is(lua_State *lua, int idx);
template<> inline bool Lua_is<bool>(lua_State *lua, int idx) { return lua_isboolean(lua, idx); }
template<> inline bool Lua_is<float>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }
template<> inline bool Lua_is<double>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }
template<> inline bool Lua_is<const char*>(lua_State *lua, int idx) { return lua_isstring(lua, idx) != 0; }
template<> inline bool Lua_is<int>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }

template<> inline unsigned Lua_to<unsigned>(lua_State *lua, int idx) { return static_cast<int>(Lua_to<int>(lua, idx)); }
template<> inline void Lua_push<unsigned>(lua_State *lua, unsigned t) { Lua_push<int>(lua, static_cast<int>(t)); }
template<> inline bool Lua_is<unsigned>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }

//----------------------------------------------------

/** INDEXES ARE INCREMENTED BY ONE AUTOMATICALLY!!! */
struct TableDispatcher {
	TableDispatcher(lua_State *lua, int index, bool DoPop = false) : m_lua(lua), m_index(0), m_DoPop(DoPop) {
		m_index = lua_gettop(lua);// +index;
		if (!lua_istable(lua, m_index)) {
			lua_newtable(lua);
			m_index = lua_gettop(lua);// +index;
			m_DoPop = true;
			//AddLog(Error, "TableDispatcher: argument is not a table!!");
		}
	}
	~TableDispatcher() {
		if (m_DoPop)
			lua_pop(m_lua, 1);
	}

	float getFloat(const char* name, float Default) { lua_pushstring(m_lua, name); return get(Default); }
	float getFloat(int index, float Default) { lua_pushinteger(m_lua, index+1); return get(Default); }
	int getInt(const char* name, int Default) { lua_pushstring(m_lua, name); return get(Default); }
	int getInt(int index, int Default) { lua_pushinteger(m_lua, index+1); return get(Default); }
	const char* getString(const char* name, const char* Default) { lua_pushstring(m_lua, name); return get(Default); }
	const char* getString(int index, const char* Default) { lua_pushinteger(m_lua, index+1); return get(Default); }
	bool getBoolean(const char* name, bool Default) { lua_pushstring(m_lua, name); return get(Default); }
	bool getBoolean(int index, bool Default) { lua_pushinteger(m_lua, index+1); return get(Default); }

#ifdef XMATH_H
	math::vec2 GetVector(const char *name, const math::vec2& Default) {
		if (IsTable(name))
			try {
				TableDispatcher td = GetTable(name);
				return math::vec2(td.getFloat(0, Default[0]), 
								  td.getFloat(1, Default[1]));
			}
			catch (...) { }
		return Default; 
	}

	math::vec3 GetVector(const char *name, const math::vec3& Default) {
		if (IsTable(name))
			try {
				TableDispatcher td = GetTable(name);
				return math::vec3(td.getFloat(0, Default[0]), 
								  td.getFloat(1, Default[1]), 
								  td.getFloat(2, Default[2]));
			}
			catch (...) { }
		return Default; 
	}

	math::vec4 GetVector(const char *name, const math::vec4& Default) {
		if (IsTable(name))
			try {
				TableDispatcher td = GetTable(name);
				return math::vec4(td.getFloat(0, Default[0]), 
								  td.getFloat(1, Default[1]), 
								  td.getFloat(2, Default[2]),
								  td.getFloat(3, Default[3]));
			}
			catch (...) { }
		return Default; 
	}
#endif

	bool ElementExists(const char *name) const {
		lua_pushstring(m_lua, name);
		lua_gettable(m_lua, m_index);
		bool ret = lua_isnil(m_lua, -1);
		lua_pop(m_lua, 1);
		return !ret;
	}

	bool ElementExists(int index) const {
		lua_pushinteger(m_lua, index + 1);
		lua_gettable(m_lua, m_index);
		bool ret = lua_isnil(m_lua, -1);
		lua_pop(m_lua, 1);
		return !ret;
	}

	struct NotALuaTable { };

	bool TableDispatcher::IsTable(const char *name) const {
		lua_pushstring(m_lua, name);
		lua_gettable(m_lua, m_index);
		bool ret = lua_istable(m_lua, -1);
		lua_pop(m_lua, 1);
		return ret;
	}

	bool TableDispatcher::IsTable(int index) const {
		lua_pushinteger(m_lua, index + 1);
		lua_gettable(m_lua, m_index);
		//stackDump(m_lua);
		bool ret = lua_istable(m_lua, -1);
		lua_pop(m_lua, 1);
		return ret;
	}

	//throws NotALuaTable in case of error!
	TableDispatcher TableDispatcher::GetTable(const char *name) {
		lua_pushstring(m_lua, name);
		lua_gettable(m_lua, m_index);
		if (!lua_istable(m_lua, -1)) {
			lua_pop(m_lua, 1);
			throw NotALuaTable();
		}
		return TableDispatcher(m_lua, lua_gettop(m_lua), true);
	}

	//throws NotALuaTable in case of error!
	TableDispatcher TableDispatcher::GetTable(int index) {
		lua_pushinteger(m_lua, index + 1);
		lua_gettable(m_lua, m_index);
		if (!lua_istable(m_lua, -1)) {
			lua_pop(m_lua, 1);
			throw NotALuaTable();
		}
		return TableDispatcher(m_lua, lua_gettop(m_lua), true);
	}
protected:
	float get(float Default) {
		lua_gettable(m_lua, m_index);
		float v;
		if(lua_isnil(m_lua, -1)) v = Default;
		else v = static_cast<float>(lua_tonumber(m_lua, -1));
		lua_pop(m_lua, 1);
		return v;
	}
	double get(double Default) {
		lua_gettable(m_lua, m_index);
		double v;
		if(lua_isnil(m_lua, -1)) v = Default;
		else v = lua_tonumber(m_lua, -1);
		lua_pop(m_lua, 1);
		return v;
	}
	int get(int Default) {
		lua_gettable(m_lua, m_index);
		int v;
		if(lua_isnil(m_lua, -1)) v = Default;
		else v = static_cast<int>(lua_tointeger(m_lua, -1));
		lua_pop(m_lua, 1);
		return v;
	}
	const char* get(const char* Default) {
		lua_gettable(m_lua, m_index);
		const char* v;
		if(lua_isnil(m_lua, -1)) v = Default;
		else v = lua_tostring(m_lua, -1);
		lua_pop(m_lua, 1);
		return v;
	}
	bool get(bool Default) {
		lua_gettable(m_lua, m_index);
		char v;
		if(lua_isnil(m_lua, -1)) v = Default;
		else v = static_cast<bool>(lua_toboolean(m_lua, -1) > 0);
		lua_pop(m_lua, 1);
		return v != 0;
	}
private:
	lua_State *m_lua;
	int m_index;
	bool m_DoPop;
};

//----------------------------------------------------

struct TableStaticCallDispatcher {
	typedef void(*Tfun)(lua_State *);
	template <void(*FunPtr)(TableDispatcher&)>
	static Tfun get() {
		return (Tfun)&DispatchCall<FunPtr>;
	}
private:
	TableStaticCallDispatcher() = delete;
	~TableStaticCallDispatcher();
	TableStaticCallDispatcher& operator=(const TableStaticCallDispatcher&) = delete;
	template <void(*FunPtr)(TableDispatcher&)>
	static int DispatchCall(lua_State *l) {
		TableDispatcher t(l, -2);
		FunPtr(t);
		return 0;
	}
};

template <class T, void(T::*FunPtr)(TableDispatcher&)>
struct TableCallDispatcher {
	typedef TableCallDispatcher<T, FunPtr> ThisClass;
	typedef int(T::*Tfun)(lua_State *);
	static Tfun get() {
		return (Tfun)&ThisClass::DispatchCall;
	}
private:
	TableCallDispatcher() = delete;
	~TableCallDispatcher();
	TableCallDispatcher& operator=(const TableCallDispatcher&) = delete;
	int DispatchCall(lua_State *l) {
		TableDispatcher t(l, -2);
		T* ptr = (T*)this;
		(ptr->*FunPtr)(t);
		return 0;
	}
};

template <class T, class Ret, Ret(T::*FunPtr)(TableDispatcher&)>
struct TableCallDispatcherEx {
	typedef TableCallDispatcherEx<T, Ret, FunPtr> ThisClass;
	typedef int(T::*Tfun)(lua_State *);

	static Tfun get() {
		return (Tfun)&ThisClass::DispatchCall;
	}
private:
	TableCallDispatcherEx() = delete;
	~TableCallDispatcherEx();
	TableCallDispatcherEx& operator=(const TableCallDispatcherEx&) = delete;
	int DispatchCall(lua_State *l) {
		TableDispatcher t(l, -2);
		T* ptr = (T*)this;
		Ret r = (ptr->*FunPtr)(t);
		luabridge::Stack<Ret>::push(l, r);
		return 1;
	}
};

//----------------------------------------------------

struct LuaStringReader {
	LuaStringReader(const std::string *string): m_String(string), m_Finished(false) {}
	static const char * Reader(lua_State *L, void *data, size_t *size) {
		return ((LuaStringReader*)data)->ReaderImpl(L, size);
	}
private:
	const char * ReaderImpl(lua_State *L, size_t *size) {
		if (m_Finished) {
			*size = 0;
			return nullptr;
		}
		m_Finished = true;
		*size = m_String->length();
		return m_String->c_str();
	}
	bool m_Finished;
	const std::string *m_String;
};

struct LuaCStringReader {
	LuaCStringReader(const char *string, size_t length): m_String(string), m_Length(length), m_Finished(false) {}
	static const char * Reader(lua_State *L, void *data, size_t *size) {
		return ((LuaCStringReader*)data)->ReaderImpl(L, size);
	}
private:
	const char * ReaderImpl(lua_State *L, size_t *size) {
		if (m_Finished) {
			*size = 0;
			return nullptr;
		}
		m_Finished = true;
		*size = m_Length;
		return m_String;
	}
	bool m_Finished;
	size_t m_Length;
	const char *m_String;
};

}
}
