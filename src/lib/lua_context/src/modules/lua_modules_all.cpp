#pragma once

#include "lua_context/modules/lua_modules_all.hpp"
#include "lua_context/modules/lua_context_module.hpp"
#include "lua_context/modules/lua_file_system.hpp"
#include "lua_context/modules/lua_print.hpp"
#include "lua_context/modules/lua_random.hpp"
#include "lua_context/modules/lua_require_handler.hpp"
#include <build_configuration.hpp>

namespace MoonGlare::Lua {

/*@ [/LuaModules] Lua modules
@*/

namespace {

void ScriptModuleManagerReady(iInterfaceHooks *interface_hooks, iScriptModuleManager *script_module_manager) {
    auto require_module = std::make_shared<LuaRequireModule>();
    script_module_manager->AddModule(require_module);

    script_module_manager->AddModule(std::make_shared<LuaPrintModule>());
    script_module_manager->AddModule(std::make_shared<LuaRandomModule>());

    interface_hooks->InstallInterfaceHook<iScriptContext>([script_module_manager](auto *script_context) {
        script_module_manager->AddModule(std::make_shared<LuaContextModule>(script_context));
    });

    interface_hooks->InstallInterfaceHook<iReadOnlyFileSystem>(
        [script_module_manager, require_module](auto *filesystem) {
            require_module->SetFileSystemInterface(filesystem);
            script_module_manager->AddModule(std::make_shared<LuaFileSystemModule>(filesystem));
        });
}

} // namespace

void LoadAllLuaModules(iInterfaceHooks *interface_hooks) {
    interface_hooks->InstallInterfaceHook<iScriptModuleManager>([interface_hooks](auto *script_module_manager) {
        ScriptModuleManagerReady(interface_hooks, script_module_manager);
    });
}

} // namespace MoonGlare::Lua
