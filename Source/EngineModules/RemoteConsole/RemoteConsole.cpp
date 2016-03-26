/*
  * Generated by cppsrc.sh
  * On 2015-06-28 11:37:08.64
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include "../MoonGlareInsider/MoonGlareInisderApi.h"

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

#include "RemoteConsole.h"

namespace MoonGlare {
namespace Debug {
namespace RemoteConsoleModule {
using namespace InsiderApi;

struct RemoteConsoleSettings {
	struct Port : public Settings_t::BaseSettingInfo<int, Port> {
		static Type default() { return Configuration::recon_Port; }
	};
	struct Enabled : public Settings_t::BaseSettingInfo<bool, Enabled> {
		static Type default() { return true; }
	};
};

struct RemoteConsoleModule : public MoonGlare::Modules::ModuleInfo {
	RemoteConsoleModule(): BaseClass("RemoteConsole", ModuleType::Debug) { 
		Settings.RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<RemoteConsoleSettings::Enabled>>("Debug.RemoteConsole.Enabled", true);
		Settings.RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<RemoteConsoleSettings::Port>>("Debug.RemoteConsole.Port", true);
	}

	virtual bool Initialize() override {
		if (RemoteConsoleSettings::Enabled::get())
			m_Instance = std::make_unique<RemoteConsole>();
		return true;
	}
	virtual bool Finalize() override {
		m_Instance.reset();
		return true;
	}
private:
	std::unique_ptr<RemoteConsole> m_Instance;
};
DEFINE_MODULE(RemoteConsoleModule);

//----------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(RemoteConsole);

RemoteConsole::RemoteConsole(): BaseClass() {
	m_Running = true;
	m_Thread = std::thread(&RemoteConsole::ThreadEntry, this);
	SetPerformanceCounterOwner(CodeExecutionCount);
}

RemoteConsole::~RemoteConsole() {
	m_Running = false;
	m_ioservice.stop();
	if(m_Thread.joinable())
		m_Thread.join(); 
}

void RemoteConsole::ThreadEntry() {
	::OrbitLogger::ThreadInfo::SetName("RECO");
	AddLog(Info, "RemoteConsole Thread started");
	EnableScriptsInThisThread();

	char buffer[Configuration::MaxMessageSize];
	auto *header = reinterpret_cast<MessageHeader*>(buffer);

	try {
		udp::socket sock(m_ioservice, udp::endpoint(udp::v4(), (unsigned short)RemoteConsoleSettings::Port::get()));

		AddLog(Hint, "Remote console initialized");
		while (m_Running) {
			//auto len = 1
			if (sock.available() <= 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			udp::endpoint remote_endpoint;
			boost::system::error_code error;
			sock.receive_from(boost::asio::buffer(buffer, sizeof(buffer) - 1), remote_endpoint, 0, error);
			//m_ioservice.run();
			//m_ioservice.reset();

			if (error && error != boost::asio::error::message_size)
				continue;
				//throw boost::system::system_error(error);

			switch (header->MessageType) {
			case MessageTypes::ExecuteCode: {
				AddLogf(Info, "Recived lua command. Size: %d bytes. Data: %s ", header->PayloadSize, header->PayLoad);
				int ret = ::Core::Scripts::ScriptProxy::ExecuteCode((char*)header->PayLoad, header->PayloadSize - 1, "RemoteConsole");
				auto *payload = reinterpret_cast<PayLoad_ExecutionResult*>(header->PayLoad);
				payload->ReturnCode = ret;
				header->MessageType = MessageTypes::ExecutionResult;
				header->PayloadSize = sizeof(PayLoad_ExecutionResult);
				break;
			}
			default:
				AddLogf(Info, "Unknown command. Size: %d bytes, type: %d ", header->PayloadSize, header->MessageType);
			}

			IncrementPerformanceCounter(CodeExecutionCount);
		}
	}
	catch (...) { }
}

} //namespace RemoteConsoleModule
} //namespace Debug
} //namespace MoonGlare
