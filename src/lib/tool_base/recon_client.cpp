
#include "recon_client.h"
#include <api/remote_console.hpp>
#include <udp_socket.h>

namespace MoonGlare::Tools::RemoteConsole {

struct ReconClient::Internals : public UdpSocket<Api::RemoteConsole::MessageBuffer> {
    void SendLine(const std::string &txt) {
        char buffer[MaxMessageSize];
        auto *header = reinterpret_cast<Api::RemoteConsole::MessageHeader *>(buffer);
        header->messageType = Api::RemoteConsole::ReconMessageType::ExecuteCode;
        char *strbase = (char *)header->payLoad;

        auto l =
            std::min(txt.length(), MaxMessageSize - sizeof(Api::RemoteConsole::MessageHeader) - 1);
        memcpy(strbase, txt.c_str(), l);
        strbase[l] = '\0';

        header->payloadSize = static_cast<uint32_t>(l) + 1;

        Send(header);
    }
};

ReconClient::ReconClient() {
    internals = std::make_unique<Internals>();
}

ReconClient::~ReconClient() {
}

void ReconClient::ConnectTo(const std::string &host, uint16_t port) {
    internals->ConnectTo(port, host);
}

void ReconClient::Send(const std::string &data) {
    internals->SendLine(data);
}

} // namespace MoonGlare::Tools::RemoteConsole
