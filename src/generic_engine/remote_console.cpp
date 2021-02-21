#if 0
#include <pch.h>

#include <nfMoonGlare.h>

#include <Core/Scripts/ScriptEngine.h>
#include <Engine/Modules/iModule.h>
#include <Foundation/Tools/RemoteConsoleApi.h>

#include <Foundation/Settings.h>
#include <interface_map.h>

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

#undef GetObject

namespace MoonGlare::Modules {

namespace RemoteConsole = MoonGlare::Tools::RemoteConsole::Api;

struct RemoteConsoleModule : public iModule {
    RemoteConsoleModule(InterfaceMap &ifaceMap) : iModule(ifaceMap) {}

    ~RemoteConsoleModule() {
        m_Running = false;
        m_ioservice.stop();
        if (m_Thread.joinable())
            m_Thread.join();
    }

    virtual std::string GetName() const { return "RemoteConsole"; };

    virtual void OnPostInit() {
        interfaceMap.GetObject(scriptEngine);

        Settings *stt = nullptr;
        interfaceMap.GetObject(stt);

        bool enabled = stt->GetBool("RemoteConsole.Enabled", DEBUG_TRUE);
        port = (uint16_t)stt->GetInt("RemoteConsole.Port", port);

        if (enabled) {
            m_Thread = std::thread(&RemoteConsoleModule::ThreadEntry, this);
        }
    }

  private:
    using udp = boost::asio::ip::udp;

    Core::Scripts::ScriptEngine *scriptEngine = nullptr;

    uint16_t port = RemoteConsole::ReconPort;

    std::thread m_Thread;
    bool m_Running;
    boost::asio::io_service m_ioservice;

    void ThreadEntry() {
        ::OrbitLogger::ThreadInfo::SetName("RECO");
        AddLog(Info, "RemoteConsole Thread started");

        char buffer[Tools::Api::MaxMessageSize];
        auto *header = reinterpret_cast<RemoteConsole::MessageHeader *>(buffer);

        udp::socket sock(m_ioservice, udp::endpoint(udp::v4(), port));

        AddLog(Hint, "Remote console initialized");
        while (m_Running) {
            try {
                if (sock.available() <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                udp::endpoint remote_endpoint;
                boost::system::error_code error;
                sock.receive_from(
                    boost::asio::buffer(buffer, sizeof(buffer) - 1),
                    remote_endpoint, 0, error);

                if (error && error != boost::asio::error::message_size)
                    continue;

                switch (header->messageType) {
                case RemoteConsole::MessageType::ExecuteCode: {
                    AddLogf(Info,
                            "Received lua command. Size: %d bytes. Data: %s ",
                            header->payloadSize, header->payLoad);
                    MoonGlare::Core::GetScriptEngine()->ExecuteCode(
                        (char *)header->payLoad, header->payloadSize - 1,
                        "RemoteConsole");
                    break;
                }
                default:
                    AddLogf(Info, "Unknown command. Size: %d bytes, type: %d ",
                            header->payloadSize, header->messageType);
                }
            } catch (...) {
                __debugbreak();
            }
        }
    }
};

#ifdef DEBUG_SCRIPTAPI
ModuleClassRegister::Register<RemoteConsoleModule>
    RemoteConsoleModuleReg("RemoteConsoleModule");
#endif

} // namespace MoonGlare::Modules

#endif