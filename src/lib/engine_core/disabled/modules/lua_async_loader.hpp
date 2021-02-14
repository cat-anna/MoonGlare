#pragma once
#if 0
#include "../iDynamicModule.h"
#include "../iLuaRequire.h"
#include <Foundation/Resources/iAsyncLoader.h>

namespace MoonGlare::Scripts::Modules {

class LuaAsyncLoaderModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaAsyncLoaderModule(lua_State *lua, InterfaceMap *world);
    ~LuaAsyncLoaderModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;

    static ApiInitializer RegisterScriptApi(ApiInitializer api);
protected:
    InterfaceMap &interfaceMap;
    Resources::iAsyncLoader::JobStatus GetStatus() const;
};

} //namespace MoonGlare::Scripts::Modules
#endif