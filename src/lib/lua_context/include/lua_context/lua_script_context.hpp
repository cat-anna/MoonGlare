#pragma once

#include <lua_context/script_module.hpp>
#include <lua_context/script_runner_interface.hpp>
#include <mutex>
#include <vector>

struct lua_State;

namespace MoonGlare::Lua {

class LuaScriptContext : public iCodeChunkRunner, public iScriptModuleManager {
public:
    LuaScriptContext();
    ~LuaScriptContext() override;

    void CollectGarbage();
    void PrintMemoryUsage() const;
    float GetMemoryUsage() const;

    bool ExecuteCode(const char *code, unsigned len, const char *CodeName = nullptr, int rets = 0);

    bool ExecuteCodeChunk(const char *code, unsigned len, const char *chunk_name = nullptr) override {
        return ExecuteCode(code, len, chunk_name, 0);
    }

    void AddModule(std::shared_ptr<iDynamicScriptModule> module) override;
    iCodeChunkRunner *GetCodeRunnerInterface() override { return this; }

private:
    lua_State *lua_state = nullptr;
    mutable std::recursive_mutex lua_state_mutex;
    int current_gc_step = 1;
    std::shared_ptr<iRequireModule> require_module;

    std::vector<std::shared_ptr<iDynamicScriptModule>> modules;

    void GcStep();
    void InitLuaState();

    auto LockLuaStateMutex() const { return std::lock_guard<std::recursive_mutex>(lua_state_mutex); }
};

} // namespace MoonGlare::Lua

#if 0

namespace Core {
namespace Scripts {

class ScriptEngine final {
public:
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

    lua_State *GetLua() { return m_Lua; }
    std::recursive_mutex &GetLuaMutex() { return m_Mutex; }

    // result will be on top of lua stack, on fail stack remain unchanged
    bool GetComponentEntryMT(SubSystemId cid);

    template <typename T> void RegisterLuaSettings(T *t, const char *Name) {
        LOCK_MUTEX_NAMED(GetLuaMutex(), lock);
        GetApiInitializer().beginNamespace("Settings").addPtrVariable(Name, t).endNamespace();
    }

    static void RegisterScriptApi(ApiInitializer &api);

protected:
    void GetComponentMTTable(lua_State *lua) {
        lua_pushlightuserdata(lua, GetComponentMTTableIndex());
        lua_gettable(lua, LUA_REGISTRYINDEX);
    }
    void *GetComponentMTTableIndex() { return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this) + 1); }

    DeclarePerformanceCounter(ExecutionCount);
    DeclarePerformanceCounter(ExecutionErrors);

};

} // namespace Scripts
} // namespace Core

#endif