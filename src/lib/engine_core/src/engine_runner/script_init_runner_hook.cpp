#pragma once

#include "engine_runner/script_init_runner_hook.hpp"
#include <fmt/format.h>

namespace MoonGlare::Runner {

void ScriptInitRunnerHook::OnContainerMounted(StarVfs::iVfsContainer *container) {
    auto file_id = container->FindFile(Lua::kLuaInitScript);
    std::string file_data;
    if (container->ReadFileContent(file_id, file_data)) {
        loaded_init_scripts.emplace_back(fmt::format("{}/{}", container->GetContainerName(), Lua::kLuaInitScript),
                                         std::move(file_data));
    }
}

void ScriptInitRunnerHook::AfterDataModulesLoad() {
    for (auto &[name, code] : loaded_init_scripts) {
        script_runner->ExecuteCodeChunk(code, name.c_str());
    }
    loaded_init_scripts.clear();
}

} // namespace MoonGlare::Runner