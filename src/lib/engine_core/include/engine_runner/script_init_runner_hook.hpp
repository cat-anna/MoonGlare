#pragma once

#include "engine_runner_hooks.hpp"
#include <lua_context/script_runner_interface.hpp>
#include <string>
#include <svfs/svfs_hooks.hpp>
#include <utility>
#include <vector>

namespace MoonGlare::Runner {

class ScriptInitRunnerHook : public StarVfs::iStarVfsHooks, public iEngineRunnerHooks {
public:
    ScriptInitRunnerHook(Lua::iCodeChunkRunner *script_runner) : script_runner(script_runner) {}
    void OnContainerMounted(StarVfs::iVfsContainer *container) override;

    void AfterDataModulesLoad();

private:
    Lua::iCodeChunkRunner *const script_runner;
    std::vector<std::pair<std::string, std::string>> loaded_init_scripts;
};

} // namespace MoonGlare::Runner