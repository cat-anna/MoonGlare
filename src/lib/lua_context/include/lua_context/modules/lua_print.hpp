#pragma once

#include "lua_context/script_module.hpp"

namespace MoonGlare::Lua {

class LuaPrintModule : public iDynamicScriptModule {
public:
    LuaPrintModule();
    ~LuaPrintModule() override = default;
    void InitContext(lua_State *lua) override;
};

} // namespace MoonGlare::Lua