#define _WIN32_WINNT 0x0502

#include "engine_runner/modules/remote_console.hpp"
#include "api/remote_console.hpp"
#include "debugger_support.hpp"
#include "lua_context/script_runner_interface.hpp"
#include <boost/asio.hpp>
#include <orbit_logger.h>
#include <thread>

// #include <Foundation/Tools/RemoteConsoleApi.h>

namespace MoonGlare::Runner::Modules {

using namespace MoonGlare::Api::RemoteConsole;

// virtual std::string GetName() const { return "RemoteConsole"; };

// virtual void OnPostInit() {
// interfaceMap.GetObject(scriptEngine);

// Settings *stt = nullptr;
// interfaceMap.GetObject(stt);

// bool enabled = stt->GetBool("RemoteConsole.Enabled", DEBUG_TRUE);
// port = (uint16_t)stt->GetInt("RemoteConsole.Port", port);

// if (enabled) {
//
// }
// }

struct RemoteConsoleModule::Impl {
    std::thread thread;
    bool running = false;
    boost::asio::io_service io_service;

    using udp = boost::asio::ip::udp;
    Lua::iCodeChunkRunner *code_runner = nullptr;

    uint16_t port = kReconPort;

    Impl() {
        running = true;
        thread = std::thread(&Impl::ThreadEntry, this);
    }
    ~Impl() {
        running = false;
        io_service.stop();
        if (thread.joinable()) {
            thread.join();
        }
    }

    void ThreadEntry() {
        ::OrbitLogger::ThreadInfo::SetName("RECO");

        char buffer[Api::MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader *>(buffer);

        udp::socket sock(io_service, udp::endpoint(udp::v4(), port));

        AddLog(Hint, "Remote console initialized");
        while (running) {
            try {
                if (sock.available() <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                udp::endpoint remote_endpoint;
                boost::system::error_code error;
                sock.receive_from(boost::asio::buffer(buffer, sizeof(buffer) - 1), remote_endpoint,
                                  0, error);

                if (error && error != boost::asio::error::message_size) {
                    continue;
                }

                switch (header->messageType) {
                case ReconMessageType::ExecuteCode: {
                    AddLogf(Info, "Received lua command. Size: %d bytes. Data:\n%s",
                            header->payloadSize, header->payLoad);

                    std::string code{(char *)header->payLoad, header->payloadSize - 1};

                    if (code_runner != nullptr) {
                        code_runner->ExecuteCodeChunk(code, "RemoteConsole");
                    }
                    break;
                }
                default:
                    AddLogf(Info, "Unknown command. Size: %d bytes, type: %d ", header->payloadSize,
                            header->messageType);
                }
            } catch (...) {
                TriggerBreakPoint();
            }
        }
        AddLog(Hint, "Remote console shutdown");
    }
};

RemoteConsoleModule::RemoteConsoleModule() : impl(std::make_unique<Impl>()) {
}

RemoteConsoleModule::~RemoteConsoleModule() = default;

void RemoteConsoleModule::InstallInterfaceHooks(iEngineRunnerHooksHost *hooks) {
    hooks->InstallInterfaceHook<Lua::iCodeChunkRunner>(&impl->code_runner);
}

} // namespace MoonGlare::Runner::Modules
