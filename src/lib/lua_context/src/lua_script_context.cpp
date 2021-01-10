
#include "lua_context/lua_script_context.hpp"
#include "lua_error_handling.hpp"
#include "lua_exec_string.hpp"
#include "modules/lua_print.hpp"
#include "modules/lua_require_handler.hpp"
#include <build_configuration.hpp>
#include <lua.hpp>
#include <orbit_logger.h>

using namespace std::string_literals;

namespace MoonGlare::Lua {

namespace {
void OpenLibs(lua_State *L) {

    static const luaL_Reg loadedlibs[] = {{"_G", luaopen_base},
                                          //    {LUA_LOADLIBNAME, luaopen_package},
                                          {LUA_COLIBNAME, luaopen_coroutine},
                                          {LUA_TABLIBNAME, luaopen_table},
                                          //    {LUA_IOLIBNAME, luaopen_io},
                                          //    {LUA_OSLIBNAME, luaopen_os},
                                          {LUA_STRLIBNAME, luaopen_string},
                                          {LUA_MATHLIBNAME, luaopen_math},
#ifdef LUA_UTF8LIBNAME
                                          {LUA_UTF8LIBNAME, luaopen_utf8},
#endif
                                          {NULL, NULL}};

    const luaL_Reg *lib;
    /* "require" functions from 'loadedlibs' and set results to global table */
    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1); /* remove lib */
    }
    if constexpr (kDebugBuild) {
        luaL_requiref(L, LUA_DBLIBNAME, luaopen_debug, 1);
        lua_pop(L, 1); /* remove lib */
    }
}

} // namespace

LuaScriptContext::LuaScriptContext(std::shared_ptr<iReadOnlyFileSystem> filesystem) {
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Script, "SCRI");
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::ScriptCall, "SCCL", false);
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::ScriptRuntime, "SCRT");

    AddLog(Debug, "Constructing lua runner");

    InitLuaState();

    AddModule(std::make_shared<LuaRequireModule>(filesystem));
    AddModule(std::make_shared<LuaPrintModule>());
}

LuaScriptContext::~LuaScriptContext() {
    auto lock = LockLuaStateMutex();
    // modules.clear();
    PrintMemoryUsage();
    lua_close(lua_state);
    lua_state = nullptr;
}

void LuaScriptContext::GcStep() {
    lua_gc(lua_state, LUA_GCSTEP, current_gc_step);

    // float memusage = GetMemoryUsage();
    // if (memusage > (m_LastMemUsage + 10.0f)) {
    //     m_LastMemUsage = memusage;
    //     ++m_CurrentGCRiseCounter;
    //     if (m_CurrentGCRiseCounter == 5) {
    //         ++m_CurrentGCStep;
    //         m_CurrentGCRiseCounter = 0;
    //         AddLogf(Debug, "New Lua GC step: %d", m_CurrentGCStep);
    //     }
    // }

    // #ifdef PERF_PERIODIC_PRINT
    //     AddLogf(Performance, "Lua memory usage: %6.2f k bytes", memusage);
    // #endif
}

void LuaScriptContext::CollectGarbage() {
    auto lock = LockLuaStateMutex();

    // #ifdef DEBUG

    float prev = GetMemoryUsage();
    lua_gc(lua_state, LUA_GCCOLLECT, 0);
    float next = GetMemoryUsage();
    AddLogf(Debug, "Finished lua garbage collection. %.2f -> %.2f kb (released %.2f kb)", prev, next, prev - next);

    // #else
    //     lua_gc(lua_state, LUA_GCCOLLECT, 0);
    // #endif
}

void LuaScriptContext::PrintMemoryUsage() const {
    AddLogf(Performance, "Lua memory usage: %.2fkb ", GetMemoryUsage());
}

float LuaScriptContext::GetMemoryUsage() const {
    return (float)lua_gc(lua_state, LUA_GCCOUNT, 0) + (float)lua_gc(lua_state, LUA_GCCOUNTB, 0) / 1024.0f;
}

void LuaScriptContext::InitLuaState() {
    AddLog(System, "Compiled with Lua version: " LUA_VERSION);

    lua_state = luaL_newstate();
    OpenLibs(lua_state);
    lua_atpanic(lua_state, LuaPanicHandler);

    // #ifdef DEBUG
    //     luabridge::setHideMetatables(false);
    // #else
    //     luabridge::setHideMetatables(true);
    // #endif

    // lua_pushlightuserdata(lua_state, GetComponentMTTableIndex()); //stack: ... index
    // lua_createtable(lua_state, 0, 0);                             //stack: ... index c-table
    // MoonGlare::Core::Scripts::PublishSelfLuaTable(lua_state, "ComponentEntryMT", this, -1);
    // lua_settable(lua_state, LUA_REGISTRYINDEX); //stack: ... index c-table

    try {
        // using namespace MoonGlare::Scripts::Modules;
        // using namespace MoonGlare::Core::Scripts::Modules;
        // ApiInit::Initialize(this);
        // lua_pushnil(lua_state);
        // lua_setglobal(lua_state, "api");
        // InstallStaticModules(lua_state);
        // StaticModules::InitStrings(lua_state, m_world);
        // StaticModules::InitApplication(lua_state, m_world);
        // StaticModules::InitThread(lua_state, m_world);
        // InstallModule<LuaSettingsModule, Settings::iLuaSettingsModule>();
        // InstallModule<LuaStaticStorageModule>();
        // InstallModule<LuaFileSystemModule>();
        // InstallModule<LuaEventsModule>();
        // InstallModule<LuaTimeModule>();
        // InstallModule<LuaAsyncLoaderModule>();
        // InstallModule<LuaDebugContextModule>();
        // InstallModule<Component::ScriptObject>();
    } catch (const std::exception &e) {
        AddLogf(Error, "Exception during static module init '%s'", e.what());
        throw;
    }

    lua_gc(lua_state, LUA_GCCOLLECT, 0);
    lua_gc(lua_state, LUA_GCSTOP, 0);
    PrintMemoryUsage();
}

bool LuaScriptContext::ExecuteCode(const char *code, unsigned len, const char *CodeName, int rets) {
    auto lock = LockLuaStateMutex();
    return ExecuteString(lua_state, code, len, CodeName, rets);
}

void LuaScriptContext::AddModule(std::shared_ptr<iDynamicScriptModule> module) {
    auto lock = LockLuaStateMutex();
    modules.emplace_back(module);
    module->InitContext(lua_state);
}

} // namespace MoonGlare::Lua

#if 0

#include <nfMoonGlare.h>

#include <Engine/Core/DataManager.h>

#include <Core/Scripts/LuaApi.h>
#include <Foundation/Scripts/ErrorHandling.h>

#include "ScriptEngine.h"

#include "Modules/LuaSettings.h"
#include "Modules/StaticModules.h"
#include "ScriptObject.h"

#include <Foundation/Scripts/Modules/LuaAsyncLoader.h>
#include <Foundation/Scripts/Modules/LuaDebugContext.h>
#include <Foundation/Scripts/Modules/LuaEvents.h>
#include <Foundation/Scripts/Modules/LuaFilesystem.h>
#include <Foundation/Scripts/Modules/LuaRequire.h>
#include <Foundation/Scripts/Modules/LuaStaticStorage.h>
#include <Foundation/Scripts/Modules/LuaTime.h>
#include <Foundation/Scripts/Modules/StaticModules.h>

#include <Core/Component/ComponentRegister.h>

namespace Core {
namespace Scripts {
using namespace MoonGlare::Scripts;

ScriptEngine* ScriptEngine::s_instance = nullptr;

RegisterApiBaseClass(ScriptEngine, &ScriptEngine::RegisterScriptApi);

ScriptEngine::ScriptEngine(World *world) : {

    SetPerformanceCounterOwner(ExecutionErrors);
    SetPerformanceCounterOwner(ExecutionCount);

    LOCK_MUTEX(m_Mutex);

    if (!ConstructLuaScriptContext()) {
        AddLog(Error, "Unable to initialize Lua context!");
        throw std::runtime_error("Unable to initialize Lua context!");
    }

    auto lua = GetLua();
    luabridge::Stack<ScriptEngine*>::push(lua, this);
    lua_setglobal(lua, "Script");

}

ScriptEngine::~ScriptEngine() {
    LOCK_MUTEX(m_Mutex);

    auto lua = GetLua();
    MoonGlare::Core::Scripts::HideSelfLuaTable(lua, "ScriptEngine", this);

    AddLog(Debug, "Destroying script object");
    if (!ReleaseLuaScriptContext()) {
        AddLog(Warning, "An error has occur during finalization of Lua context!");
    }

    AddLog(Debug, "Destruction finished");
}

//---------------------------------------------------------------------------------------

void ScriptEngine::RegisterScriptApi(ApiInitializer &root) {
    root
    .beginClass<ThisClass>("ScriptEngine")
#ifdef DEBUG_SCRIPTAPI
        .addFunction("CollectGarbage", &ThisClass::CollectGarbage)
        .addFunction("PrintMemoryUsage", &ThisClass::PrintMemoryUsage)
#endif
    .endClass()
    .beginClass<MoveConfig>("cMoveConfig")
        .addData("TimeDelta", &MoveConfig::timeDelta, false)
        .addData("SecondPeriod", &MoveConfig::m_SecondPeriod, false)
    .endClass();
}

//---------------------------------------------------------------------------------------

void ScriptEngine::Step(const MoveConfig &config) {
    LOCK_MUTEX(m_Mutex);

    Modules::StaticModules::ThreadStep(lua_state, m_world);
}

//---------------------------------------------------------------------------------------

bool ScriptEngine::GetComponentEntryMT(SubSystemId cid) {
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

    auto uptr = std::make_unique<Class>(lua_state, m_world);
    m_world->SetInterface<Iface>(uptr.get());
    mod.basePtr = std::move(uptr);

    if (auto regFunc = MoonGlare::Scripts::GetApiInitFunc<Class>(); regFunc) {
        luabridge::getGlobalNamespace(lua_state)
            .beginNamespace("api")
                .DefferCalls(regFunc)
            .endNamespace()
            ;
    }

#ifdef DEBUG_SCRIPTAPI
    if (auto regFunc = MoonGlare::Scripts::GetDebugApiInitFunc<Class>(); regFunc) {
        luabridge::getGlobalNamespace(lua_state)
            .beginNamespace("debug")
                .DefferCalls(regFunc)
            .endNamespace()
            ;
    }
#endif
}

} //namespace Scripts
} //namespace Core

#endif