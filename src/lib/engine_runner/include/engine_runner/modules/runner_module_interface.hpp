#pragma once

#include "engine_runner/engine_runner_hooks.hpp"

namespace MoonGlare::Runner::Modules {

class iRunnerModule {
public:
    virtual ~iRunnerModule() = default;
    virtual void InstallInterfaceHooks(iEngineRunnerHooksHost *hooks) = 0;
};

} // namespace MoonGlare::Runner::Modules
