#pragma once

#include <memory>
#include <string>

namespace MoonGlare::Tools::RemoteConsole {

struct ReconClient {
    ReconClient();
    ~ReconClient();

    void ConnectTo(const std::string &host, uint16_t port);
    void Send(const std::string &data);
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

}
