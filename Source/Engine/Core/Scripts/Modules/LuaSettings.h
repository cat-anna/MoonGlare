#pragma once

#include "../iLuaSettings.h"

#include <EngineBase/Scripts/iLuaRequire.h>
#include <EngineBase/Scripts/iDynamicModule.h>

namespace MoonGlare::Core::Scripts::Modules {
using namespace MoonGlare::Scripts;

class LuaSettingsModule : public iDynamicScriptModule, public Settings::iLuaSettingsModule, public iRequireRequest {
public:
    LuaSettingsModule(lua_State *lua, World *world);
    ~LuaSettingsModule() override;

    void RegisterProvider(std::string prefix, Settings::iSettingsProvider *provider) override;
    bool OnRequire(lua_State *lua, std::string_view name) override;
protected:
    World *world;
    struct SettingsObject;

    std::unordered_map<std::string, Settings::iSettingsProvider *> providerMap;

    bool scriptApiRegistered = false;

    void RegisterScriptApi(lua_State *lua);
};

} //namespace MoonGlare::Core::Script::Modules
