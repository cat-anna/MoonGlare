#pragma once

#include <Foundation/Scripts/ExecuteCode.h>
#include <Foundation/Scripts/iDynamicModule.h>

namespace MoonGlare {
namespace Core {
namespace Scripts {

using namespace MoonGlare::Scripts;

class ScriptEngine final {
    using ThisClass = ScriptEngine;

public:
    static ScriptEngine *s_instance;
    static void DeleteInstance() {
        delete s_instance;
        s_instance = nullptr;
    }

    ScriptEngine(World *world);
    virtual ~ScriptEngine();

    void Step(const MoveConfig &conf);

    template <typename Iface> Iface *QueryModule() const {
        auto it = modules.find(std::type_index(typeid(Iface)));
        if (it == modules.end()) {
            AddLogf(Error, "Module %s is not registered", typeid(Iface).name());
            return nullptr;
        }
        return dynamic_cast<Iface *>(it->second.basePtr.get());
    }

    ApiInitializer GetApiInitializer() { return luabridge::getGlobalNamespace(m_Lua); }

    bool ExecuteCode(const char *code, unsigned len, const char *CodeName = nullptr, int rets = 0) {
        LOCK_MUTEX(m_Mutex);
        return MoonGlare::Scripts::ExecuteString(m_Lua, code, len, CodeName, rets);
    }
    bool ExecuteCode(const std::string &code, const char *CodeName = nullptr, int rets = 0) {
        return ExecuteCode(code.c_str(), code.length(), CodeName, rets);
    }

    lua_State *GetLua() { return m_Lua; }
    std::recursive_mutex &GetLuaMutex() { return m_Mutex; }

    // result will be on top of lua stack, on fail stack remain unchanged
    bool GetComponentEntryMT(SubSystemId cid);

    template <typename T> void RegisterLuaSettings(T *t, const char *Name) {
        LOCK_MUTEX_NAMED(GetLuaMutex(), lock);
        GetApiInitializer().beginNamespace("Settings").addPtrVariable(Name, t).endNamespace();
    }

    void CollectGarbage();
    void PrintMemoryUsage() const;
    float GetMemoryUsage() const;

    static void RegisterScriptApi(ApiInitializer &api);

protected:
    bool ConstructLuaContext();
    bool ReleaseLuaContext();

    void GetComponentMTTable(lua_State *lua) {
        lua_pushlightuserdata(lua, GetComponentMTTableIndex());
        lua_gettable(lua, LUA_REGISTRYINDEX);
    }
    void *GetComponentMTTableIndex() { return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this) + 1); }

    DeclarePerformanceCounter(ExecutionCount);
    DeclarePerformanceCounter(ExecutionErrors);

private:
    lua_State *m_Lua = nullptr;
    World *m_world;
    mutable std::recursive_mutex m_Mutex;

    struct DynamicModule {
        std::unique_ptr<iDynamicScriptModule> basePtr;
    };
    std::unordered_map<std::type_index, DynamicModule> modules;

    template <typename Class, typename Iface = Class> void InstallModule();

    // old:
protected:
    int m_CurrentGCStep;
    int m_CurrentGCRiseCounter;
    float m_LastMemUsage;
};

} // namespace Scripts

inline Scripts::ScriptEngine *GetScriptEngine() { return Scripts::ScriptEngine::s_instance; }

} // namespace Core
} // namespace MoonGlare
