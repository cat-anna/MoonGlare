#include "RemoteConsoleModule.h"

#include <Foundation/Tools/RemoteConsoleApi.h>

namespace MoonGlare::Module {

namespace Api = Tools::RemoteConsole::Api;

RemoteConsole::RemoteConsole(SharedModuleManager modmgr) : iModule(std::move(modmgr)), port(Api::ReconPort) {
    SetSettingID("RemoteConsole");
}

RemoteConsole::~RemoteConsole() {
}

bool RemoteConsole::PostInit() {
    ConnectTo(host, port);
    return true;
}

bool RemoteConsole::Finalize() {
    return true;
}

bool RemoteConsole::DoSaveSettings(pugi::xml_node node) const {
    node.remove_child("Port");
    node.remove_child("Host");

    node.append_child("Port").text() = port;
    node.append_child("Host").text() = host.c_str();
    return true;
}

bool RemoteConsole::DoLoadSettings(const pugi::xml_node node) {
    port = static_cast<uint16_t>(node.child("Port").text().as_uint(Api::ReconPort));
    host = node.child("Host").text().as_string("localhost");
    return true;
}

} //namespace MoonGlare 
