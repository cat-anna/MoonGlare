/*
  * Generated by cppsrc.sh
  * On 2015-06-28 11:37:08.64
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Modules/ModuleManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/iApplication.h>

#define __LOG_ACTION_Tool(T, A)						ORBITLOGGER_BeginLog(None, A)
#define __LOG_ACTION_F_Tool(T, ...)					ORBITLOGGER_BeginLogf(None, __VA_ARGS__)
#define __LOG_ACTION_Recon(T, A)					ORBITLOGGER_BeginLog(T, A)
#define __LOG_ACTION_F_Recon(T, ...)				ORBITLOGGER_BeginLogf(T, __VA_ARGS__)

#include "MoonGlareInisder.h"
#include "ResourceEnumerator.h"
#include <Engine/iSoundEngine.h>
#include <Config/DebugInterface.h>

#include <Core/Scripts/ScriptEngine.h>

namespace MoonGlare {
namespace Debug {
namespace Insider {

using namespace InsiderApi;

struct InsiderModule : public MoonGlare::Modules::ModuleInfo {
	InsiderModule(): BaseClass("InsiderModule") { 
	}

	virtual bool Initialize() override {
		m_Instance = std::make_unique<Insider>();
		return true;
	}
	virtual bool Finalize() override {
		m_Instance.reset();
		return true;
	}

	virtual void Notify(SettingsGroup what) override {
		if (!m_Instance)
			return;
		InsiderMessageBuffer msg;
		auto hdr = msg.GetHeader();
		hdr->MessageType = MessageTypes::NotificationSettingsChanged;
		auto ptr = msg.AllocAndZero<PayLoad_NotificationSettingsChanged>();
		ptr->Group = (u32)what;
		m_Instance->SendInsiderMessage(msg);
	}
private:
	std::unique_ptr<Insider> m_Instance;
};

DEFINE_MODULE(InsiderModule);

//----------------------------------------------------------------

SPACERTTI_IMPLEMENT_STATIC_CLASS(Insider);

Insider::Insider(): BaseClass() {
	m_Running = true;
	m_Thread = std::thread(&Insider::ThreadEntry, this);
	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Insider, "INSI");
	SetPerformanceCounterOwner(CodeExecutionCount);
	SetPerformanceCounterOwner(CommandExecutionCount);
	SetPerformanceCounterOwner(BytesSend);
	SetPerformanceCounterOwner(BytesRecived);
}

Insider::~Insider() {
	m_Running = false;
	m_ioservice.stop();
	if(m_Thread.joinable())
		m_Thread.join(); 
}

bool Insider::Command(InsiderMessageBuffer& buffer, const udp::endpoint &sender) {
	IncrementPerformanceCounter(CommandExecutionCount);
	auto *header = buffer.GetHeader();
	switch (header->MessageType) {
	case MessageTypes::ExecuteCode: return ExecuteCode(buffer);
	case MessageTypes::EnumerateLua: return EnumerateLua(buffer);
	case MessageTypes::EnumerateScripts: return EnumerateScripts(buffer);
	case MessageTypes::EnumerateAudio: return EnumerateAudio(buffer);
#ifdef DEBUG_INTERFACE
	case MessageTypes::EnumerateMemory: return EnumerateMemory(buffer);
#endif
	case MessageTypes::EnumerateEntities: return EnumerateEntities(buffer);
	case MessageTypes::SetScriptCode: return SetScriptCode(buffer);
	case MessageTypes::GetScriptCode: return GetScriptCode(buffer);
	case MessageTypes::InfoRequest: return InfoRequest(buffer);
	case MessageTypes::Ping: return Ping(buffer);
	case MessageTypes::OrbitLoggerStateRequest: return OrbitLoggerState(buffer);

	case MessageTypes::Bind:
		m_Connected = true;
		m_ConnectedAddress = sender;
		//m_InsiderLogSink = std::make_unique<InsiderLogSink>(this);
		return false;
	case MessageTypes::Release:
		m_Connected = false;
		return false;

	default:
		AddLogf(Insider, "Unknown command. Size: %d bytes, type: %d ", header->PayloadSize, header->MessageType);
		buffer.Clear();
		buffer.GetHeader()->MessageType = MessageTypes::NotSupported;
		return true;
	}
}

void Insider::SendInsiderMessage(InsiderMessageBuffer& buffer) {
	if (!m_Connected) {
		AddLogf(Insider, "Unable to send message to Insider, reason: not connected. (type: %d, length:%d)", buffer.GetHeader()->MessageType, buffer.UsedSize());
		return;
	}
	boost::system::error_code ignored_error;
	buffer.GetHeader()->PayloadSize = buffer.PayLoadSize();
	LOCK_MUTEX(m_SocketMutex);
	auto sendb = m_socket->send_to(boost::asio::buffer(buffer.GetBuffer(), buffer.UsedSize()), m_ConnectedAddress, 0, ignored_error);
	PerformanceCounter_inc(BytesSend, sendb);
}

void Insider::ThreadEntry() {
	std::this_thread::sleep_for(std::chrono::seconds(2));//let the engine start

	OrbitLogger::ThreadInfo::SetName("INSI", true);
	AddLog(Info, "Insider thread started");

	InsiderMessageBuffer buffer;
	buffer.Fill(0);

	try {
		m_socket.reset(new udp::socket(m_ioservice, udp::endpoint(udp::v4(), (unsigned short)Configuration::Insider_Port)));
		
		AddLog(Insider, "Insider initialized");
		while (m_Running) {
			if (m_socket->available() <= 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			udp::endpoint remote_endpoint;
			boost::system::error_code error;
			LOCK_MUTEX(m_SocketMutex);
			auto recvb = m_socket->receive_from(boost::asio::buffer((char*)buffer.GetBuffer(), buffer.Size), remote_endpoint, 0, error);
			PerformanceCounter_inc(BytesRecived, recvb);

			if (error && error != boost::asio::error::message_size)
				continue;
				//throw boost::system::system_error(error);
			try {
				if (!Command(buffer, remote_endpoint))
					continue;
			}
			catch (...) {
				AddLogf(Error, "Unknown command error!");
			}
			boost::system::error_code ignored_error;
			buffer.GetHeader()->PayloadSize = buffer.PayLoadSize();
			auto sendb = m_socket->send_to(boost::asio::buffer(buffer.GetBuffer(), buffer.UsedSize()), remote_endpoint, 0, ignored_error);
			PerformanceCounter_inc(BytesSend, sendb);
		}
	}
	catch (...) { 
		AddLogf(Error, "Insider thread failed");
	}
}

//----------------------------------------------------------------

bool Insider::EnumerateLua(InsiderMessageBuffer& buffer) {
	auto *request = buffer.GetAndPull<PayLoad_EnumerateRequest>();

	std::stringstream ss;
	ss <<
		"function InsiderDynamicFunction(...)\n"
		"	return Insider.EnumerateLuaTable(" << request->Path << ", ...)\n"
		"end\n";

	AddLogf(Insider, "Lua enum path: %s", request->Path);

	std::string code = ss.str();
	//int ret = 
	
	Core::GetScriptEngine()->ExecuteCode(code, "InisiderEnumerator");

	ResourceEnumerator enumerator(buffer, MessageTypes::EnumerateLua);
	Core::GetScriptEngine()->RunFunction<int>("InsiderDynamicFunction", &enumerator);
	enumerator.finish();
	return true;
}

bool Insider::EnumerateScripts(InsiderMessageBuffer & buffer) {
	//auto *request = buffer.GetAndPull<??>();
	//No payload
	return false;
//	AddLogf(Insider, "Enumerating scripts");
//
//	buffer.Clear();
//	auto *list = buffer.Alloc<PayLoad_ListBase>();
//	
//	unsigned count = 0;
//	using ScriptCode = MoonGlare::Core::Scripts::ScriptEngine::ScriptCode;
//	Core::GetScriptEngine()->EnumerateScripts([&count, &buffer](const ScriptCode &code) {
//		auto *item = buffer.Alloc<PayLoad_ScriptList_Item>();
//		item->DataLen = (u16)code.Data.length();
//		item->Index = (u16)count;
//		++count;
//		item->Type = code.Type == ScriptCode::Source::Code ? PayLoad_ScriptList_Item::ItemType::BroadcastedCode : PayLoad_ScriptList_Item::ItemType::ScriptFile;
//		buffer.PushString(code.Name);
//	});
//
//	list->Count = count;
//	auto *hdr = buffer.GetHeader();
//	hdr->MessageType = MessageTypes::ScriptList;
//	return true;
}

bool Insider::EnumerateAudio(InsiderMessageBuffer& buffer) {
	//auto *request = buffer.GetAndPull<??>();
	//No payload

	AddLogf(Insider, "Enumerating scripts");

	buffer.Clear();
	auto *list = buffer.Alloc<PayLoad_ListBase>();

	u16 count = 0;
	using SoundType = MoonGlare::Sound::SoundType;
	GetSoundEngine()->EnumerateAudio([&count, &buffer](const string& Name, const string& Class, SoundType Type) {
		auto *item = buffer.Alloc<PayLoad_AudioListItem>();
		item->Index = count;
		++count;
		item->Type = Type == SoundType::Sound ? PayLoad_AudioListItem::AudioType::Sound : PayLoad_AudioListItem::AudioType::Music;
		item->NameLen = (u16)Name.length();
		item->ClassNameLen = (u16)Class.length();
		buffer.PushString(Name);
		buffer.PushString(Class);
	});

	list->Count = count;
	auto *hdr = buffer.GetHeader();
	hdr->MessageType = MessageTypes::AudioList;
	return true;
}

bool Insider::ExecuteCode(InsiderMessageBuffer& buffer) {
	auto *header = buffer.GetHeader();
	IncrementPerformanceCounter(CodeExecutionCount);
	AddLogf(Insider, "Received lua command. Size: %d bytes. Data: %s ", header->PayloadSize, header->PayLoad);
	int ret = Core::GetScriptEngine()->ExecuteCode((char*)header->PayLoad, header->PayloadSize - 1, "RemoteConsole");
	buffer.Clear();
	auto *payload = buffer.Alloc<PayLoad_ExecutionResult>();
	payload->ReturnCode = ret;
	header->MessageType = MessageTypes::ExecutionResult;
	return true;
}

bool Insider::SetScriptCode(InsiderMessageBuffer& buffer) {
//	auto *request = buffer.GetAndPull<PayLoad_SetScriptCode>();
	std::string name = buffer.PullString();
	std::string data = buffer.PullString();

//	bool saveFile = request->OverwriteContainerFile > 0;

	Core::GetScriptEngine()->ExecuteCode(data, name.c_str());

	buffer.Clear();
	buffer.GetHeader()->MessageType = MessageTypes::Ok;

	AddLogf(Insider, "Set script '%s' succeed", name.c_str());

//	if (saveFile) {
//		auto f = GetFileSystem()->OpenFileForWrite(name);
//		if (f) {
//			f->SetFileData(data.c_str(), data.length());
//			AddLogf(Warning, "Script file '%s' has been overwritten", name.c_str());
//		} else
//			AddLogf(Warning, "Failed to save script file '%s'", name.c_str());
//	}

	AddLog(Error, "NOT IMPLEMENTED");

	return true;
}

bool Insider::GetScriptCode(InsiderMessageBuffer& buffer) {
	return false;
//	auto *request = buffer.GetAndPull<PayLoad_GetScriptCode>();
//	auto name = string(request->Name, request->NameLength);
//
//	buffer.Clear();
//	bool found = false;
//
//	using ScriptCode = MoonGlare::Core::Scripts::ScriptEngine::ScriptCode;
//	Core::GetScriptEngine()->EnumerateScripts([&name, &buffer, &found](const ScriptCode &code) {
//		if (!found && name == code.Name) {
//			auto *response = buffer.Alloc<PayLoad_ScriptCode>();
//			response->DataLength = (u16)code.Data.length();
//			buffer.PushString(code.Data);
//			found = true;
//		}
//	});
//
//	if (!found) {
//		auto *response = buffer.Alloc<PayLoad_ScriptCode>();
//		response->DataLength = 0;
//		buffer.PushString("");
//	}
//
//	buffer.GetHeader()->MessageType = MessageTypes::ScriptCode;
//
//	if (found)
//		AddLogf(Insider, "Get script '%s' succeed", name.c_str());
//	else
//		AddLogf(Error, "Get script '%s' failed: no such script", name.c_str());
//
//	return true;
}

bool Insider::InfoRequest(InsiderMessageBuffer& buffer) {
	AddLogf(Debug, "Sending engine info");

	buffer.Clear();
	auto *info = buffer.Alloc<PayLoad_InfoResponse>();

    string ver = Core::VersionString;

	info->VersionLength = (u16)ver.length();
	buffer.PushString(ver);

	auto exen = GetApplication()->ExeName();
	info->ExeNameLength = (u16)strlen(exen);
	buffer.PushString(exen);

	info->BuildDateLength = (u16)strlen(Core::CompilationDate);
	buffer.PushString(Core::CompilationDate);

	auto *hdr = buffer.GetHeader();
	hdr->MessageType = MessageTypes::InfoResponse;
	return true;
}

bool Insider::Ping(InsiderMessageBuffer& buffer) {
	AddLogf(Insider, "Insider Ping");
	buffer.Clear();
	auto *hdr = buffer.GetHeader();
	hdr->MessageType = MessageTypes::Pong;
	return true;
}

bool Insider::EnumerateMemory(InsiderMessageBuffer& buffer) {
	AddLogf(Insider, "Enumerating memory pools");
	buffer.Clear();
	auto *hdr = buffer.GetHeader();

	auto *list = buffer.Alloc<PayLoad_ListBase>();
	u16 count = 0;

#ifdef DEBUG_MEMORY
	{
		auto dbgmem = Config::Current::DebugMemoryInterface::GetFirstDebugMemoryInterface();
		auto it = dbgmem.second;

		while (it) {
			for (auto counterinfo : it->DebugMemoryGetCounters()) {
				auto *item = buffer.Alloc<PayLoad_MemoryStatus>();
				item->Index = ++count;

				Config::Current::DebugMemoryInterface::DebugMemoryCounter counter = {};
				counterinfo.m_Function(counter);

				item->Allocated = counter.Allocated;
				item->Capacity = counter.Capacity;
				item->ElementSize = static_cast<u16>(counter.ElementSize);
				item->NameLen = static_cast<u16>(counterinfo.m_Name.length());
				item->OwnerNameLen = static_cast<u16>(it->DebugMemoryGetClassName().length());
				item->ID = reinterpret_cast<u64>(it);
				item->ParentID = reinterpret_cast<u64>(it->DebugMemoryGetParent());
				buffer.PushString(counterinfo.m_Name);
				buffer.PushString(it->DebugMemoryGetClassName());
			}
			it = it->GetNext();
		}
	}
#endif

	list->Count = count;
	hdr->MessageType = MessageTypes::MemoryStatusList;
	return true;
}

bool Insider::EnumerateEntities(InsiderMessageBuffer& buffer) {
	buffer.Clear();
	auto *hdr = buffer.GetHeader();

	auto em = Core::GetEngine()->GetWorld()->GetEntityManager();

	auto *list = buffer.AllocAndZero<PayLoad_ListBase>();
	auto &count = list->Count;

	for (size_t it = 0, jt = MoonGlare::Configuration::Entity::IndexLimit; it < jt; ++it) {
		auto flags = em->m_Flags[it];
		if (!flags.m_Map.m_Valid)
			continue;

		++count;
		auto info = buffer.AllocAndZero<PayLoad_EntityInfo>();

		info->Flags 			  = flags.m_UIntValue;
		info->SelfEntity		  = em->m_EntityValues[it].m_IntegerValue;
		info->ParentEntity		  = em->m_Parent[it].m_IntegerValue;
		info->FirstChildEntity	  = em->m_FirstChild[it].m_IntegerValue;
		info->NextSiblingEntity	  = em->m_NextSibling[it].m_IntegerValue;
		info->PrevSiblingEntity	  = em->m_PrevSibling[it].m_IntegerValue;

		buffer.PushString(em->m_Names[it]);
	}

	hdr->MessageType = MessageTypes::EntitiesList;
	return true;
}

bool Insider::OrbitLoggerState(InsiderMessageBuffer& buffer) {
	buffer.Clear();
	auto *hdr = buffer.GetHeader();

	auto data = buffer.AllocAndZero<PayLoad_OrbitLoggerStateResponse>();
	OrbitLogger::LogCollector::ChannelInfoTable table;

	if (OrbitLogger::LogCollector::GetChannelInfo(table)) {
		for (size_t i = 0; i < OrbitLogger::LogChannels::MaxLogChannels; ++i) {
			auto &ch = table[i];
			auto &info = data->m_Table[i];

			info.Enabled = ch.m_Enabled ? 1 : 0;
			info.Channel = ch.m_Channel;
			info.LinesPushed = ch.m_LinesPushed;
			strncpy(info.Name, ch.m_Name, sizeof(info.Name));
		}
	}

	hdr->MessageType = MessageTypes::OrbitLoggerStateResponse;
	return true;
}

} //namespace Insider
} //namespace Debug
} //namespace MoonGlare