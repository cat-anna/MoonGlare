
#include "ReconClient.h"

#include <Foundation/Tools/RemoteConsoleApi.h>
#include <Foundation/Tools/UdpSocket.h>

namespace MoonGlare::Tools::RemoteConsole {

struct ReconClient::Internals : public UdpSocket<Api::MessageBuffer> {
    void SendLine(const std::string &txt) {
        char buffer[MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader*>(buffer);
        header->messageType = MessageType::ExecuteCode;
        char *strbase = (char*)header->payLoad;

        auto l = std::min(txt.length(), MaxMessageSize - sizeof(MessageHeader) - 1);
        memcpy(strbase, txt.c_str(), l);
        strbase[l] = '\0';

        header->payloadSize = static_cast<u32>(l) + 1;

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

}
