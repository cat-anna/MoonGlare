#pragma once

#include <lua_context/script_module.hpp>
#include <readonly_file_system.h>

namespace MoonGlare::Lua {

void LoadAllLuaModules(iScriptModuleManager *script_module_manager, std::shared_ptr<iReadOnlyFileSystem> filesystem);

} // namespace MoonGlare::Lua