#include "core_lua_modules.hpp"
#include "engine_runner/engine_runner_interface.hpp"
#include "engine_runner/engine_time.hpp"
#include "modules/lua_application_module.hpp"
#include "modules/lua_scenes_manager.hpp"
#include "modules/lua_time.hpp"

namespace MoonGlare::LuaModules {

namespace {

void ScriptModuleManagerReady(iInterfaceHooks *interface_hooks, Lua::iScriptModuleManager *script_module_manager) {

    interface_hooks->InstallInterfaceHook<iEngineRunner>([script_module_manager](auto *runner) {
        script_module_manager->AddModule(std::make_shared<LuaApplicationModule>(runner));
    });

    interface_hooks->InstallInterfaceHook<iEngineTime>([script_module_manager](auto *time) {
        script_module_manager->AddModule(std::make_shared<LuaTimeModule>(time));
    });

    interface_hooks->InstallInterfaceHook<SceneManager::iScenesManager>([script_module_manager](auto *scene_manager) {
        script_module_manager->AddModule(std::make_shared<SceneManagerLuaModule>(scene_manager));
    });
}

} // namespace

void LoadAllCoreLuaModules(iInterfaceHooks *interface_hooks) {
    interface_hooks->InstallInterfaceHook<Lua::iScriptModuleManager>([interface_hooks](auto *script_module_manager) {
        ScriptModuleManagerReady(interface_hooks, script_module_manager);
    });
}

} // namespace MoonGlare::LuaModules
