#pragma once

#include "runner_module_interface.hpp"

namespace MoonGlare::Runner::Modules {

struct RemoteConsoleModule : public iRunnerModule {
    RemoteConsoleModule();
    ~RemoteConsoleModule() override;

    void InstallInterfaceHooks(iEngineRunnerHooksHost *hooks) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace MoonGlare::Runner::Modules
