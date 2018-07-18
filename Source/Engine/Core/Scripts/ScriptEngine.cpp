#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>

#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/LuaReader.h>
#include <Core/Scripts/LuaApi.h>

#include "ScriptEngine.h"

#include "Modules/StaticModules.h"
#include "Modules/LuaRequire.h"
#include "Modules/LuaSettings.h"

namespace MoonGlare {
namespace Core {
namespace Scripts {
using namespace MoonGlare::Scripts;

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(ScriptEngine)
RegisterApiInstance(ScriptEngine, &ScriptEngine::Instance, "ScriptEngine");
RegisterApiDerivedClass(ScriptEngine, &ScriptEngine::RegisterScriptApi);

ScriptEngine::ScriptEngine(World *world) :
        cRootClass(),
        m_CurrentGCStep(1),
        m_CurrentGCRiseCounter(0),
        m_LastMemUsage(0) {
    SetThisAsInstance();

    assert(world);
    m_world = world;

    SetPerformanceCounterOwner(ExecutionErrors);
    SetPerformanceCounterOwner(ExecutionCount);

    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Script, "SCRI");
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::ScriptCall, "SCCL", false);
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::ScriptRuntime, "SCRT");
}

ScriptEngine::~ScriptEngine() {
}

//---------------------------------------------------------------------------------------

void ScriptEngine::RegisterScriptApi(ApiInitializer &root) {
    root
    .deriveClass<ThisClass, BaseClass>("ScriptEngine")
#ifdef DEBUG_SCRIPTAPI
        .addFunction("CollectGarbage", &ThisClass::CollectGarbage)
        .addFunction("PrintMemoryUsage", &ThisClass::PrintMemoryUsage)
#endif
    .endClass()
    .beginClass<MoveConfig>("cMoveConfig")
        .addData("TimeDelta", &MoveConfig::TimeDelta, false)
        .addData("SecondPeriod", &MoveConfig::m_SecondPeriod, false)
    .endClass();
}

//---------------------------------------------------------------------------------------

bool ScriptEngine::Initialize() {
    MoonGlareAssert(!m_Lua);

    LOCK_MUTEX(m_Mutex);

    AddLog(Debug, "Constructing script object");
    if (!ConstructLuaContext()) {
        AddLog(Error, "Unable to initialize Lua context!");
        return false;
    }

    auto lua = GetLua();
    luabridge::Stack<ScriptEngine*>::push(lua, this);
    lua_setglobal(lua, "Script");

    AddLog(Debug, "Script construction finished");
    return true;
}

bool ScriptEngine::Finalize() {
    MoonGlareAssert(m_Lua != nullptr);

    LOCK_MUTEX(m_Mutex);

    auto lua = GetLua();
    MoonGlare::Core::Scripts::HideSelfLuaTable(lua, "ScriptEngine", this);

    AddLog(Debug, "Destroying script object");
    if (!ReleaseLuaContext()) {
        AddLog(Warning, "An error has occur during finalization of Lua context!");
    }
    
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
    lua_atpanic(m_Lua, LuaPanicHandler);

    lua_newtable(m_Lua);
    lua_setglobal(m_Lua, "global");

#ifdef DEBUG
    luabridge::setHideMetatables(false);
#else
    luabridge::setHideMetatables(true);
#endif

    lua_pushlightuserdata(m_Lua, GetComponentMTTableIndex());	//stack: ... index
    lua_createtable(m_Lua, 0, 0);								//stack: ... index ctable
    MoonGlare::Core::Scripts::PublishSelfLuaTable(m_Lua, "ComponentEntryMT", this, -1);

    for (auto &cit : Component::ComponentRegister::GetComponentMap()) {
        auto &cinfo = *cit.second;
        if (!cinfo.m_EntryMetamethods)
            continue;
        auto &emt = *cinfo.m_EntryMetamethods;

        if (!emt)
            continue;
        
        if (!emt.m_Index || !emt.m_NewIndex) {
            AddLogf(Error, "Invalid component mt set!");
            continue;
        }

        lua_pushinteger(m_Lua, static_cast<int>(cinfo.m_CID));	//stack: ... index ctable cid cmt
        lua_createtable(m_Lua, 0, 0);							//stack: ... index ctable cid cmt

        if (emt.m_Index) {
            lua_pushcclosure(m_Lua, emt.m_Index, 0);
            lua_setfield(m_Lua, -2, "__index");
        }
        if (emt.m_NewIndex) {
            lua_pushcclosure(m_Lua, emt.m_NewIndex, 0);
            lua_setfield(m_Lua, -2, "__newindex");
        }
            
        lua_settable(m_Lua, -3);								//stack: ... index ctable 
    }
    lua_settable(m_Lua, LUA_REGISTRYINDEX);						//stack: ... index ctable

    try {
        Modules::StaticModules::InitStrings(m_Lua, m_world);
        Modules::StaticModules::InitApplication(m_Lua, m_world);
        Modules::StaticModules::InitPrint(m_Lua, m_world);
        Modules::StaticModules::InitMath(m_Lua, m_world);
        Modules::StaticModules::InitRandom(m_Lua, m_world);
        Modules::StaticModules::InitTime(m_Lua, m_world);
        Modules::StaticModules::InitThread(m_Lua, m_world);

        InstallModule<Modules::LuaRequireModule, iRequireModule>();
        InstallModule<Modules::LuaSettingsModule, Settings::iLuaSettingsModule>();
    }
    catch (const std::exception &e) {
        AddLogf(Error, "Exception during static module init '%s'", e.what());
        return false;
    }

    ApiInit::Initialize(this);

    lua_gc(m_Lua, LUA_GCCOLLECT, 0);
    lua_gc(m_Lua, LUA_GCSTOP, 0);
    PrintMemoryUsage();

    return true;
}

bool ScriptEngine::ReleaseLuaContext() {
    LOCK_MUTEX(m_Mutex);
    modules.clear();
    PrintMemoryUsage();
    lua_close(m_Lua);
    m_Lua = nullptr;
    return true;
}

//---------------------------------------------------------------------------------------

void ScriptEngine::Step(const MoveConfig &config) {
    LOCK_MUTEX(m_Mutex);

    Modules::StaticModules::ThreadStep(m_Lua, m_world);
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

bool ScriptEngine::Call(lua_State *lua, int args, int rets) {
    switch (lua_pcall(lua, args, rets, 0)) {
    case 0:
        return true;
    case LUA_ERRRUN:
        AddLog(Error, "Call error: a runtime error!");
        break;
    case LUA_ERRERR:                    
        AddLog(Error, "Call error: error while running the error handler function!");
        break;
    case LUA_ERRMEM:
        AddLog(Error, "Call error: Memory allocation failed!");
        break;
    }
    AddLog(Error, "Call error message: " << lua_tostring(lua, -1));
    lua_pop(lua, 1);
    return false;
}

bool ScriptEngine::ExecuteCode(lua_State *lua, const char* Code, unsigned len, const char* ChunkName, int rets) {
    MoonGlareAssert(Code);

    LuaCStringReader reader(Code, len);
    int result = lua_load(lua, &reader.Reader, &reader, ChunkName);

    switch (result) {
    case 0:
        AddLogf(Debug, "Loaded lua chunk: '%s'", ChunkName ? ChunkName : "?");
        return Call(lua, 0, rets);
    case LUA_ERRSYNTAX:
        AddLogf(Error, "Unable to load script: Syntax Error!\nName:'%s'\nError string: '%s'\ncode: [[%s]]", ChunkName ? ChunkName : "?", lua_tostring(lua, -1), Code);
        break;
    case LUA_ERRMEM:
        AddLog(Error, "Unable to load script: Memory allocation failed!");
        break;
    }

    lua_pop(lua, 1);
    return false;
}

//---------------------------------------------------------------------------------------

bool ScriptEngine::GetComponentEntryMT(ComponentID cid) {
    auto lua = GetLua();
    GetComponentMTTable(lua);

    lua_pushinteger(lua, static_cast<int>(cid));
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

//---------------------------------------------------------------------------------------

template<typename Class, typename Iface>
void ScriptEngine::InstallModule() {
    auto &mod = modules[std::type_index(typeid(Iface))];
    if (mod.basePtr) {
        throw std::runtime_error(std::string("Attempt to re-register iface ") + typeid(Iface).name());
    }

    mod.basePtr = std::make_unique<Class>(m_Lua, m_world);
}

} //namespace Scripts
} //namespace Core
} //namespace MoonGlare
