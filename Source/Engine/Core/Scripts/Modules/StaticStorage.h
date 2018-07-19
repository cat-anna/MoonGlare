#pragma once

#include "../iDynamicModule.h"
#include "../iLuaSettings.h"
#include "../iLuaRequire.h"

namespace MoonGlare::Core::Scripts::Modules {

class StaticStorageModule : public iDynamicScriptModule, public iRequireRequest {
public:
    StaticStorageModule(lua_State *lua, World *world);
    ~StaticStorageModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;
protected:
    ScriptEngine *scriptEngine;

    void SaveStorage();
};

} //namespace MoonGlare::Core::Script::Modules
