#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <luasupport.h>
#include <memory>
#include <sol/sol.hpp>

namespace MoonGlare::Tools::VfsCli {

struct LuaDeleter {
    void operator()(lua_State *l) { lua_close(l); }
};

struct Lua;
using SharedLua = std::shared_ptr<Lua>;

struct Lua {
    static SharedLua New() {
        struct L : public Lua {};
        return std::make_shared<L>();
    }

    lua_State *GetState() { return sol_lua.lua_state(); }
    sol::state_view &get_sol() { return sol_lua; }

    bool Initialize();

    bool LoadLibrary(const char *c);
    bool ExecuteScriptFile(const char *fname);
    bool ExecuteScriptChunk(const char *code, const char *name = nullptr);
    bool ExecuteChunk(const unsigned char *data, size_t len, const char *name);

protected:
    Lua();
    virtual ~Lua() {}

    void RegisterAPI();

private:
    sol::state sol_lua;
};

} // namespace MoonGlare::Tools::VfsCli
