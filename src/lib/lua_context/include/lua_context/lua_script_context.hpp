#pragma once

#include "static_timer.hpp"
#include <lua_context/script_module.hpp>
#include <lua_context/script_runner_interface.hpp>
#include <mutex>
#include <vector>

struct lua_State;

namespace MoonGlare::Lua {

class LuaScriptContext final : public iScriptContext, public iCodeChunkRunner, public iScriptModuleManager {
public:
    LuaScriptContext();
    ~LuaScriptContext() override;

    void PrintMemoryUsage() const;

    bool ExecuteCode(const char *code, unsigned len, const char *CodeName = nullptr, int rets = 0);

    bool ExecuteCodeChunk(const char *code, unsigned len, const char *chunk_name = nullptr) override {
        return ExecuteCode(code, len, chunk_name, 0);
    }

    void AddModule(std::shared_ptr<iDynamicScriptModule> module) override;

    void CollectGarbage() override;
    void SetGcStepSize(int size) override { current_gc_step = size; }
    int GetGcStepSize() const override { return current_gc_step; }
    void GcStep() override;
    void Step(double dt) override;
    float GetMemoryUsage() const override;

    iCodeChunkRunner *GetCodeRunnerInterface() override { return this; }
    iScriptModuleManager *GetModuleManager() override { return this; }

private:
    lua_State *lua_state = nullptr;
    int current_gc_step = 1;
    mutable std::recursive_mutex lua_state_mutex;
    std::shared_ptr<iRequireModule> require_module;

    std::vector<std::shared_ptr<iDynamicScriptModule>> modules;

    StaticTimer stat_timer{std::chrono::seconds(10)};

    void InitLuaState();

    auto LockLuaStateMutex() const { return std::lock_guard<std::recursive_mutex>(lua_state_mutex); }
};

} // namespace MoonGlare::Lua
