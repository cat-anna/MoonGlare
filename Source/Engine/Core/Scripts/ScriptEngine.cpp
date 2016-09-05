#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include "GlobalContext.h"

#include "LuaUtils.h"
#include <Utils/LuaUtils.h>

namespace MoonGlare {
namespace Core {
namespace Scripts {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(ScriptEngine)
RegisterApiInstance(ScriptEngine, &ScriptEngine::Instance, "ScriptEngine");
RegisterApiDerivedClass(ScriptEngine, &ScriptEngine::RegisterScriptApi);

ScriptEngine::ScriptEngine() :
		cRootClass(),
		m_CurrentGCStep(1),
		m_CurrentGCRiseCounter(0),
		m_LastMemUsage(0) {
	SetThisAsInstance();

	SetPerformanceCounterOwner(ExecutionErrors);
	SetPerformanceCounterOwner(ExecutionCount);

	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Script, "SCRI");
	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::ScriptCall, "SCCL", false);
}

ScriptEngine::~ScriptEngine() {
}

//---------------------------------------------------------------------------------------

void ScriptEngine::Step(const MoveConfig &config) {
	LOCK_MUTEX(m_Mutex);

	lua_gc(m_Lua, LUA_GCSTEP, m_CurrentGCStep);

	if (!config.m_SecondPeriod) {
		return;
	}

	float memusage = GetMemoryUsage();
	if (memusage > (m_LastMemUsage + 10.0f) ) {
		m_LastMemUsage = memusage;
		++m_CurrentGCRiseCounter;
		if (m_CurrentGCRiseCounter == 5) {
			++m_CurrentGCStep;
			m_CurrentGCRiseCounter = 0;
			AddLogf(Debug, "New Lua GC step: %d", m_CurrentGCStep);
		}
	}

#ifdef PERF_PERIODIC_PRINT
	AddLogf(Performance, "Lua memory usage: %6.2f kbytes", memusage);
#endif
}

//---------------------------------------------------------------------------------------

void ScriptEngine::RegisterScriptApi(ApiInitializer &root) {
	root
	.deriveClass<ThisClass, BaseClass>("ScriptEngine")
		.addCFunction("New", &ThisClass::RegisterNewScript)
		.addCFunction("Modify", &ThisClass::RegisterModifyScript)
#ifdef DEBUG_SCRIPTAPI
		.addFunction("CollectGarbage", &ThisClass::CollectGarbage)
		.addFunction("PrintMemoryInfo", &ThisClass::PrintMemoryInfo)
#endif
	.endClass()
	.beginClass<MoveConfig>("cMoveConfig")
		.addData("TimeDelta", &MoveConfig::TimeDelta, false)
		.addData("SecondPeriod", &MoveConfig::m_SecondPeriod, false)
	.endClass();
}

//---------------------------------------------------------------------------------------

void ScriptEngine::CollectGarbage() {
	LOCK_MUTEX(m_Mutex);
#ifdef DEBUG
	float prev = GetMemoryUsage();
	lua_gc(m_Lua, LUA_GCCOLLECT, 0);
	float next = GetMemoryUsage();
	AddLogf(Debug, "Finished lua garbage collection. %.2f -> %.2f kb (released %.2f kb)", prev, next, prev - next);
#else
	lua_gc(m_Lua, LUA_GCCOLLECT, 0);
#endif
}

void ScriptEngine::PrintMemoryInfo() {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	AddLogf(Performance, "Lua memory usage: %.2fkb ", GetMemoryUsage());
#endif
}

//---------------------------------------------------------------------------------------

bool ScriptEngine::Initialize() {
	MoonGlareAssert(!m_Lua);

	new ::Core::Scripts::GlobalContext();
	::Core::Scripts::GlobalContext::Instance()->Initialize();

	LOCK_MUTEX(m_Mutex);

	AddLog(Debug, "Constructing script object");
	if (!ConstructLuaContext()) {
		AddLog(Error, "Unable to initialize Lua context!");
		return false;
	}

	for (auto &it : m_ScriptCodeList) {
		LoadCode(it.Data.c_str(), it.Data.length(), it.Name.c_str());
	}

	auto lua = GetLua();
	luabridge::Stack<ScriptEngine*>::push(lua, this);
	lua_setglobal(lua, "Script");

	lua_pushlightuserdata(lua, (void *)this);  
	lua_createtable(lua, 0, 0);
#if DEBUG
	lua_pushvalue(lua, -1);
	char name[64];
	sprintf_s(name, "ScriptEngine_%p", this);
	lua_setglobal(lua, name);
	AddLogf(Debug, "Adding global registry mapping: %s by %p(%s)", name, this, typeid(*this).name());
#endif
	lua_settable(lua, LUA_REGISTRYINDEX);

	AddLog(Debug, "Script construction finished");
	return true;
}

bool ScriptEngine::Finalize() {
	MoonGlareAssert(m_Lua != nullptr);

	LOCK_MUTEX(m_Mutex);

	auto lua = GetLua();
#if DEBUG
	lua_pushnil(lua);
	char name[64];
	sprintf_s(name, "ScriptEngine_%p", this);
	lua_setglobal(lua, name);
	AddLogf(Debug, "Deleting mapped global: %s by %p(%s)", name, this, typeid(*this).name());
#endif

	AddLog(Debug, "Destroying script object");
	if (!ReleaseLuaContext()) {
		AddLog(Warning, "An error has occur during finalization of Lua context!");
	}

	::Core::Scripts::GlobalContext::Instance()->Finalize();
	::Core::Scripts::GlobalContext::DeleteInstance();

	AddLog(Debug, "Destruction finished");
	return true;
}

bool ScriptEngine::ConstructLuaContext() {
	MoonGlareAssert(m_Lua == nullptr);

	m_Lua = luaL_newstate();
	luaopen_base(m_Lua);
	luaopen_math(m_Lua);
	luaopen_bit(m_Lua);
	luaopen_string(m_Lua);
	luaopen_table(m_Lua);
#ifdef DEBUG
	luaopen_debug(m_Lua);
#endif
	lua_atpanic(m_Lua, LuaPanic);

	lua_newtable(m_Lua);
	lua_setglobal(m_Lua, "global");

#ifdef DEBUG
	luabridge::setHideMetatables(false);
#else
	luabridge::setHideMetatables(true);
#endif

	ApiInit::Initialize(this);

	::Core::Scripts::GlobalContext::Instance()->Install(m_Lua);

	lua_gc(m_Lua, LUA_GCCOLLECT, 0);
	lua_gc(m_Lua, LUA_GCSTOP, 0);
	PrintMemoryUsage();

	return true;
}

bool ScriptEngine::ReleaseLuaContext() {
	LOCK_MUTEX(m_Mutex);
	PrintMemoryUsage();
	lua_close(m_Lua);
	m_Lua = nullptr;
	return true;
}

//---------------------------------------------------------------------------------------

void ScriptEngine::LoadAllScriptsImpl() {
	FileSystem::FileInfoTable files;
	if (!GetFileSystem()->EnumerateFolder(DataPath::Scripts, files, true)) {
		AddLog(Error, "Unable to look for scripts!");
	}

	using FileInfo = MoonGlare::FileSystem::FileInfo;
	std::sort(files.begin(), files.end(), [](const FileInfo &a, const FileInfo &b) -> bool{
		return a.m_RelativeFileName < b.m_RelativeFileName;
	});

	for (auto &it : files){
		if (it.m_IsFolder)
			continue;

		string path;
		FileSystem::DataSubPaths.Translate(path, it.m_RelativeFileName, DataPath::Scripts);
		RegisterScript(path);
	}

	GetDataMgr()->NotifyResourcesChanged();
	AddLog(Debug, "Finished looking for scripts");
}

void ScriptEngine::LoadAllScripts() {
//	JobQueue::QueueJob([this]() { 
		LoadAllScriptsImpl(); 
	//});
}

void ScriptEngine::RegisterScript(string Name) {
	ScriptCode *last;
	{
		ScriptCode sc_value;
		sc_value.Name.swap(Name);
		sc_value.Type = ScriptCode::Source::File;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc_value));
		last = &m_ScriptCodeList.back();
	}

	ScriptCode &sc = *last;
	
	if (sc.Data.empty()) {
		StarVFS::ByteTable data;
		if (!GetFileSystem()->OpenFile(sc.Name, DataPath::Root, data)) {
			AddLog(Warning, "Unable to open script file " << sc.Name);
			return;
		}
		sc.Data = string(data.get(), data.size());
		AddLogf(Debug, "Loaded script code: %s", sc.Name.c_str());
	}

	try {
		LoadCode(sc.Data.c_str(), sc.Data.length(), sc.Name.c_str());
	}
	catch (...) {
		AddLog(Error, "Unexpected exception while loading script code!");
	}
}

void ScriptEngine::SetCode(const string& ChunkName, string Code) {
	ScriptCode *item = nullptr;
	LOCK_MUTEX(m_Mutex);
	{
		for (auto &it : m_ScriptCodeList) {
			if (it.Name == ChunkName) {
				item = &it;
				break;
			}
		}

		if(!item) {
			AddLogf(Debug, "Chunk '%s' not found. Creating new", ChunkName.c_str());
			ScriptCode sc;
			sc.Name = ChunkName;
			sc.Type = ScriptCode::Source::Code;
			m_ScriptCodeList.emplace_back(std::move(sc));
			item = &m_ScriptCodeList.back();
		}
	}

	ScriptCode &sc = *item;
	sc.Data.swap(Code);
	AddLogf(Debug, "Exchanged code for chunk '%s'", ChunkName.c_str());
}

//---------------------------------------------------------------------------------------

void ScriptEngine::LoadCode(string code) {
	ScriptCode *last;
	{
		ScriptCode sc;
		sc.Type = ScriptCode::Source::Code;
		sc.Data.swap(code);
		char buffer[64];
		sprintf(buffer, "Code_%d", m_ScriptCodeList.size());
		sc.Name = buffer;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc));
		last = &m_ScriptCodeList.back();
	}

	try {
		LOCK_MUTEX(m_Mutex);
		LoadCode(last->Data.c_str(), last->Data.length(), last->Name.c_str());
	}
	catch (...) {
		AddLog(Error, "Unexpected exception while loading script code!");
	}

	GetDataMgr()->NotifyResourcesChanged();
}

int ScriptEngine::LoadCode(const char* Code, unsigned len, const char* ChunkName) {
	LOCK_MUTEX(m_Mutex);

	Utils::Scripts::LuaCStringReader reader(Code, len);

	int result = lua_load(m_Lua, &reader.Reader, &reader, ChunkName);

	switch (result) {
	case 0:
		if (lua_pcall(m_Lua, 0, 0, 0) != 0) {
			AddLog(Error, "Lua error: " << lua_tostring(m_Lua, -1));
			//throw std::runtime_error();
			return -1;
		}
		break;
	case LUA_ERRSYNTAX:
		AddLogf(Error, "Unable to load script: Syntax Error!\nName:'%s'\nError string: '%s'\ncode: [[%s]]", ChunkName, lua_tostring(m_Lua, -1), Code);
		break;
	case LUA_ERRMEM:
		AddLog(Error, "Unable to load script: Memory allocation failed!");
		break;
	}

	return result;
}

//---------------------------------------------------------------------------------------

bool ScriptEngine::GetRegisteredScript(const char *name) {
	if (!name) {
		return false;
	}

	auto lua = GetLua();
	lua_pushlightuserdata(lua, (void *)this);
	lua_gettable(lua, LUA_REGISTRYINDEX);  

	lua_pushstring(lua, name);
	lua_gettable(lua, -2);

	if (lua_isnil(lua, -1)) {
		lua_pop(lua, 2);
		return false;
	} else {
		lua_insert(lua, -2);
		lua_pop(lua, 1);
		return true;
	}
}

int ScriptEngine::RegisterNewScript(lua_State * lua) {
	const char *name = lua_tostring(lua, -1);
	if (!name) {
		AddLog(Error, "Attempt to register nameless script!");
		return 0;
	}

	//Utils::Scripts::LuaStackOverflowAssert check(lua);
	lua_pushlightuserdata(lua, (void *)this);
	lua_gettable(lua, LUA_REGISTRYINDEX); 

	lua_getfield(lua, -1, name);

	if (!lua_isnil(lua, -1)) {
#ifdef DEBUG
		AddLogf(Info, "Modifying script: %s", name);

		lua_getfield(lua, -1, "__index");

		lua_insert(lua, -3);

		lua_pop(lua, 2);

		return 1;
#else
		AddLogf(Error, "Attempt to redefine script: %s", name);
		lua_pop(lua, 2);
		return 0;
#endif
	} else {
		AddLogf(Info, "Registering script: %s", name);
		lua_pop(lua, 1);
		lua_createtable(lua, 0, 0);
		lua_pushvalue(lua, -1);
		lua_setfield(lua, -3, name);

		lua_insert(lua, -2);
		lua_pop(lua, 1);

		lua_pushstring(lua, name);
		lua_setfield(lua, -2, "Name");

		lua_createtable(lua, 0, 0);
		lua_pushvalue(lua, -1);
		lua_setfield(lua, -3, "__index");

		lua_insert(lua, -2);

		lua_pop(lua, 1);

		return 1;
	}
}

int ScriptEngine::RegisterModifyScript(lua_State * lua) {
	const char *name = lua_tostring(lua, -1);
	if (!name) {
		AddLog(Error, "Attempt to register nameless script!");
		return 0;
	}

	//Utils::Scripts::LuaStackOverflowAssert check(lua);
	lua_pushlightuserdata(lua, (void *)this);
	lua_gettable(lua, LUA_REGISTRYINDEX);

	lua_getfield(lua, -1, name);

	if (lua_isnil(lua, -1)) {
		AddLogf(Error, "Attempt to define script: %s", name);
		lua_pop(lua, 2);
		return 0;
	} else {
		//TODO: Test this!
		AddLogf(Info, "Modifying script: %s", name);

		lua_getfield(lua, -1, "__index");

		lua_insert(lua, -3);

		lua_pop(lua, 2);

		return 1;
	}
}

//---------------------------------------------------------------------------------------

void ScriptEngine::PrintMemoryUsage() const {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	AddLogf(Performance, "Lua memory usage: %.2fkb ", GetMemoryUsage());
#endif
}

float ScriptEngine::GetMemoryUsage() const {
	LOCK_MUTEX(m_Mutex);
	return (float)lua_gc(m_Lua, LUA_GCCOUNT, 0) + (float)lua_gc(m_Lua, LUA_GCCOUNTB, 0) / 1024.0f;
}

//---------------------------------------------------------------------------------------

void ScriptEngine::EnumerateScripts(EnumerateFunc func) {
	LOCK_MUTEX(m_Mutex);
	for (auto &it : m_ScriptCodeList)
		func(it);
}

//---------------------------------------------------------------------------------------

#ifdef DEBUG_DUMP
void ScriptEngine::DumpScripts(std::ostream &out) {
	out << "Registered scripts:\n";
	{
		LOCK_MUTEX(m_Mutex);
		for (auto &it : m_ScriptCodeList) {
			out << "\t" << it.Name << "  length: " << it.Data.length() << " bytes\n";
		}
	}
	out << "\n";
}
#endif

} //namespace Scripts
} //namespace Core
} //namespace MoonGlare
