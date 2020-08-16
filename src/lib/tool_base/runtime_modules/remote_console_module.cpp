#include "remote_console_module.hpp"
#include <api/remote_console.h>

namespace MoonGlare::Tools::RuntineModules {

namespace Api = Tools::RemoteConsole::Api;

RemoteConsoleModule::RemoteConsoleModule(SharedModuleManager modmgr)
    : iModule(std::move(modmgr)), port(Api::ReconPort) {
    // SetSettingID("RemoteConsoleModule");
}

RemoteConsoleModule::~RemoteConsoleModule() {}

bool RemoteConsoleModule::PostInit() {
    // ConnectTo(host, port);
    return true;
}

bool RemoteConsoleModule::Finalize() { return true; }

// bool RemoteConsoleModule::DoSaveSettings(pugi::xml_node node) const {
//     node.remove_child("Port");
//     node.remove_child("Host");

//     node.append_child("Port").text() = port;
//     node.append_child("Host").text() = host.c_str();
//     return true;
// }

// bool RemoteConsoleModule::DoLoadSettings(const pugi::xml_node node) {
//     port = static_cast<uint16_t>(node.child("Port").text().as_uint(Api::ReconPort));
//     host = node.child("Host").text().as_string("localhost");
//     return true;
// }

} // namespace MoonGlare::Tools::RuntineModules
