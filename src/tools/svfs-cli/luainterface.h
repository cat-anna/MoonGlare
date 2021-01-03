#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include "arguments.h"
#include <luasupport.h>
#include <memory>
#include <runtime_modules.h>
#include <sol/sol.hpp>

namespace MoonGlare::Tools::VfsCli {

struct LuaDeleter {
    void operator()(lua_State *l) { lua_close(l); }
};

struct Lua;
using SharedLua = std::shared_ptr<Lua>;

struct Lua {
    static SharedLua New(const InitEnv &env) {
        struct L : public Lua {
            L(const InitEnv &env) : Lua(env) {}
        };
        return std::make_shared<L>(env);
    }

    lua_State *GetState() { return sol_lua.lua_state(); }
    sol::state_view &get_sol() { return sol_lua; }

    bool Initialize(SharedModuleManager module_manager);

    bool LoadLibrary(const char *c);
    bool ExecuteScriptFile(const char *fname);
    bool ExecuteScriptChunk(const char *code, const char *name = nullptr);
    bool ExecuteChunk(const unsigned char *data, size_t len, const char *name);

protected:
    Lua(const InitEnv &env);
    virtual ~Lua() {}

    void RegisterAPI();

private:
    sol::state sol_lua;
};

} // namespace MoonGlare::Tools::VfsCli
