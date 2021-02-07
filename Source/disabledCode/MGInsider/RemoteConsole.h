/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RemoteConsole_H
#define RemoteConsole_H

#include <Shared/MoonGlareInsider/Api.h>
namespace InsiderApi = MoonGlare::Debug::InsiderApi;
#include "RemoteConsoleObserver.h"

class RemoteConsoleObserver;
using SharedRemoteConsoleObserver = std::shared_ptr < RemoteConsoleObserver >;

enum class RemoteConsoleState {
	Unknown,
	NotStarted,
	Starting,
	Ready,
	Working,
	Broken,
	Stopping,
};

class RemoteConsole : public QObject {
	Q_OBJECT;
public:
	static RemoteConsole* get();
	RemoteConsoleState GetState();
	void Delete();

	void RequestTimedout(SharedRemoteConsoleObserver observer);
	void MakeRequest(SharedRemoteConsoleObserver observer);
	void ExecuteCode(const QString &code); 
signals:
	void StateChanged(RemoteConsoleState state);
protected:
 	RemoteConsole();
 	~RemoteConsole();

	bool Initialize();
	bool Finalize();

	void SetState(RemoteConsoleState st);
	
	void ProcessMessage(InsiderApi::InsiderMessageBuffer &buffer);
	void ProcessSelfMessage(InsiderApi::InsiderMessageBuffer &buffer);
private slots:
	void DataReady();
	void Tick();
private: 
	std::unique_ptr<QTimer> m_TickTimer;
	struct Impl;
	std::unique_ptr<Impl> m_Impl;
};

inline RemoteConsole& GetRemoteConsole() { return *RemoteConsole::get(); }

#endif