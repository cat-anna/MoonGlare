#pragma once

#include <ToolBase/Module.h>
#include <ToolBase/Tools/ReconClient.h>

#include "iSettingsUser.h"

namespace MoonGlare::Module {

class RemoteConsole : public iModule, public iSettingsUser, public Tools::RemoteConsole::ReconClient {
public:
    RemoteConsole(SharedModuleManager modmgr);
    ~RemoteConsole();

    bool PostInit() override;
    bool Finalize() override;

    bool DoSaveSettings(pugi::xml_node node) const override;
    bool DoLoadSettings(const pugi::xml_node node) override;
private:
    uint16_t port = 0;
    std::string host = "localhost";
};

} //namespace MoonGlare
