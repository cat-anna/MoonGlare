#pragma once

#include "lua_context/modules/lua_modules_all.hpp"
#include "lua_context/modules/lua_file_system.hpp"
#include "lua_context/modules/lua_print.hpp"
#include "lua_context/modules/lua_random.hpp"
#include "lua_context/modules/lua_require_handler.hpp"
#include <build_configuration.hpp>

namespace MoonGlare::Lua {

/*@ [/LuaModules] Lua modules
@*/

namespace {

void FileSystemReady(iScriptModuleManager *script_module_manager, iReadOnlyFileSystem *filesystem) {
    //must be first
    script_module_manager->AddModule(std::make_shared<LuaRequireModule>(filesystem));

    script_module_manager->AddModule(std::make_shared<LuaPrintModule>());
    script_module_manager->AddModule(std::make_shared<LuaFileSystemModule>(filesystem));
    script_module_manager->AddModule(std::make_shared<LuaRandomModule>());
}

void ScriptModuleManagerReady(iInterfaceHooks *interface_hooks, iScriptModuleManager *script_module_manager) {
    interface_hooks->InstallInterfaceHook<iReadOnlyFileSystem>(
        [script_module_manager](auto *filesystem) { FileSystemReady(script_module_manager, filesystem); });
}

} // namespace

void LoadAllLuaModules(iInterfaceHooks *interface_hooks) {
    interface_hooks->InstallInterfaceHook<iScriptModuleManager>([interface_hooks](auto *script_module_manager) {
        ScriptModuleManagerReady(interface_hooks, script_module_manager);
    });
}

} // namespace MoonGlare::Lua
