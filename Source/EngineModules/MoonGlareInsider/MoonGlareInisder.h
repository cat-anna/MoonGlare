/*
  * Generated by cppsrc.sh
  * On 2015-06-28 11:37:08.64
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RemoteConsole_H
#define RemoteConsole_H

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

#include <Foundation/MoonGlareInsider.h>

namespace MoonGlare {
namespace Debug {
namespace Insider {

using namespace InsiderApi;
using InsiderMessageBuffer = InsiderApi::InsiderMessageBuffer;

//using InsiderLogSink = ::Log::LogSink < > ;

class Insider : public cRootClass {
	SPACERTTI_DECLARE_STATIC_CLASS(Insider, cRootClass);
public:
 	Insider();
 	virtual ~Insider();
	
	void SendInsiderMessage(InsiderMessageBuffer& buffer);
private: 
	using udp = boost::asio::ip::udp;
	using tcp = boost::asio::ip::tcp;

	DeclarePerformanceCounter(CodeExecutionCount);
	DeclarePerformanceCounter(CommandExecutionCount);
	DeclarePerformanceCounter(BytesSend);
	DeclarePerformanceCounter(BytesRecived);
	std::thread m_Thread;
	volatile bool m_Running;
	boost::asio::io_service m_ioservice;
	std::unique_ptr<udp::socket> m_socket;
	std::mutex m_SocketMutex;

	udp::endpoint m_ConnectedAddress;
	bool m_Connected;

	/** Return false to skip sending response */
	bool Command(InsiderMessageBuffer& buffer, const udp::endpoint &sender);
	void ThreadEntry();
	
	bool ExecuteCode(InsiderMessageBuffer& buffer);
	bool EnumerateLua(InsiderMessageBuffer& buffer);
	bool EnumerateScripts(InsiderMessageBuffer& buffer);
	bool SetScriptCode(InsiderMessageBuffer& buffer);
	bool GetScriptCode(InsiderMessageBuffer& buffer);
	bool InfoRequest(InsiderMessageBuffer& buffer);
	bool Ping(InsiderMessageBuffer& buffer);
	bool EnumerateAudio(InsiderMessageBuffer& buffer);
	bool EnumerateMemory(InsiderMessageBuffer& buffer);
	bool EnumerateEntities(InsiderMessageBuffer& buffer);
	bool OrbitLoggerState(InsiderMessageBuffer& buffer);
}; 

} //namespace Insider
} //namespace Debug
} //namespace MoonGlare

#endif
