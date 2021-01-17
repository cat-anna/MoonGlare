#pragma once

#include "lua_context/modules/lua_modules_all.hpp"
#include "lua_context/modules/lua_file_system.hpp"
#include "lua_context/modules/lua_print.hpp"
#include "lua_context/modules/lua_random.hpp"
#include "lua_context/modules/lua_require_handler.hpp"
#include <build_configuration.hpp>

namespace MoonGlare::Lua {

void LoadAllLuaModules(iScriptModuleManager *script_module_manager, std::shared_ptr<iReadOnlyFileSystem> filesystem) {
    //must be first
    script_module_manager->AddModule(std::make_shared<LuaRequireModule>(filesystem));

    script_module_manager->AddModule(std::make_shared<LuaPrintModule>());
    script_module_manager->AddModule(std::make_shared<LuaFileSystemModule>(filesystem));
    script_module_manager->AddModule(std::make_shared<LuaRandomModule>());
}

} // namespace MoonGlare::Lua