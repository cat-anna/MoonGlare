#pragma once

#include "../iLuaSettings.h"

#include <Foundation/Scripts/iLuaRequire.h>
#include <Foundation/Scripts/iDynamicModule.h>

namespace MoonGlare::Core::Scripts::Modules {

using namespace MoonGlare::Scripts;


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
