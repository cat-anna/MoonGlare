#pragma once

#include "arguments.h"
#include "luainterface.h"

namespace MoonGlare::Tools::VfsCli {

class CLI {
public:
    CLI(SharedLua Lua);
    CLI(const CLI &) = delete;

    CLI &operator=(const CLI &) = delete;

    bool Enter(InitEnv &env);

private:
    SharedLua m_Lua;
    bool can_continue;
    bool bash_mode;

    bool InstallApi();
    void Loop();

    // for lua
    void Exit(int ec);

    int loadline(lua_State *L);
    int pushline(lua_State *L, int firstline);
    int multiline(lua_State *L);
};

} // namespace MoonGlare::Tools::VfsCli
