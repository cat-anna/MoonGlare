#pragma once

// #include "settings_user.h"
#include <recon_client.h>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class RemoteConsoleModule : public iModule,
                            //  public iSettingsUser,
                            public Tools::RemoteConsole::ReconClient {
public:
    RemoteConsoleModule(SharedModuleManager modmgr);
    ~RemoteConsoleModule();

    bool PostInit() override;
    bool Finalize() override;

    // bool DoSaveSettings(pugi::xml_node node) const override;
    // bool DoLoadSettings(const pugi::xml_node node) override;

private:
    uint16_t port = 0;
    std::string host = "127.0.0.1";
};

} // namespace MoonGlare::Tools::RuntineModules
