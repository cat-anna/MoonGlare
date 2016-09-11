#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iApplication.h>
#include "GameApplication.h"

//#include <libs/src/utils/ParamParser.cpp>
using namespace std;
using namespace MoonGlare::Application;

void flag_disableMouseUnhook() {
	//Instance->SetFlags(AppFlag_AllowMouseUnHook, false);
}
void flag_HideConsole() {
	//Instance->SetFlags(AppFlag_ConsoleVisible, false);
}
void flag_DisableConsole() {
	//Settings->Window.EnableConsole = false;
}
void option_setFPS(StringVector &arglist){
	//unsigned c = atoi(arglist[0].c_str());
	//Instance->SetGoalFPS(c);
} 
void option_setWidth(StringVector &arglist){
	//unsigned c = atoi(arglist[0].c_str());
	//Settings->Window.Width = c;
}
void option_setHeight(StringVector &arglist){
	//unsigned c = atoi(arglist[0].c_str());
	//Settings->Window.Height = c;
}

const Space::ProgramParameters::Parameter Parameters[] = {
//	{'s', 1, 0, 			SettingsFile,	"Select settings file."},
// 	{'', 0, f, 0, ""},

	//{'m', 0, 0, flag_disableMouseUnhook, "Disable mouse unhook", 0},
 	//{'v', 0, 0, flag_HideConsole, "Hide console", 0},
 //	{'d', 0, 0, flag_DisableConsole, "Disable console", 0},

 	//{'f', 1, 0, option_setFPS, "Set desired FPS", 0},
// 	{'x', 1, 0, option_setWidth, "Set window width", 0},
// 	{'y', 1, 0, option_setHeight, "Set window height", 0},

	{'\0', 0, 0, 0, 0, 0},
}; 

Space::ProgramParameters Params = {
	Space::ProgramParameters::disable_helpmsg_on_noparams,
	"MoonGlare Engine",
	0,//	Settings->Modules.List,
	"MainModule OtherModules",
	Parameters,
	0,
	0,
}; 

#ifndef _BUILDING_TOOLS_

int main(int argc, char** argv) {
	bool Result = false;
#pragma warning ( suppress: 4244 )
	srand(time(NULL));

	using OrbitLogger::LogCollector;
	using OrbitLogger::StdFileLoggerSink;
	using OrbitLogger::StdNoDebugFileLoggerSink;

	OrbitLogger::ThreadInfo::SetName("MAIN", true);
	LogCollector::Start();
	LogCollector::OpenLogSink<StdFileLoggerSink>([](StdFileLoggerSink* sink) { sink->Open("logs/Engine.log"); });
	LogCollector::OpenLogSink<StdFileLoggerSink>([](StdFileLoggerSink* sink) { sink->Open("logs/Engine.last.log", false); });
	LogCollector::OpenLogSink<StdNoDebugFileLoggerSink>([](StdNoDebugFileLoggerSink* sink) { sink->Open("logs/Engine.filtered.log"); });
	LogCollector::SetCaptureStdOutAndErr(OrbitLogger::LogChannels::StdOut, OrbitLogger::LogChannels::StdErr);
	LogCollector::SetChannelName(OrbitLogger::LogChannels::StdOut, "SOUT");
	LogCollector::SetChannelName(OrbitLogger::LogChannels::StdErr, "SERR");

	Config::Current::Initialize();
	AddLog(Info, "MainThread");
	//Core::Settings->DisableSave();
	do {
		bool doRestart = false;
		try {
			Params.Parse(argc, argv);
			auto app = new GameApplication(argc, argv);

			Result = false;
			Result = app->Execute();

			doRestart = app->DoRestart();
		}
		catch (const char * Msg) {
			AddLogf(Error, "FATAL ERROR! '%s'", Msg);
		}
		catch (const string & Msg) {
			AddLogf(Error, "FATAL ERROR! '%s'", Msg.c_str());
		}
		catch (std::exception &E) {
			AddLog(Error, "FATAL ERROR! '" << E.what() << "'");
		}
		catch (...) {
			AddLog(Error, "UNKNOWN FATAL ERROR!");
		}
		iApplication::DeleteInstance();
#ifdef DEBUG
		Config::Current::DumpAll("exit");
		if (Result)
			Config::Current::CheckInstances();
#endif
		if (!doRestart)
			break;
		AddLog(Debug, "Performing application restart");
	} while (true);

	Config::Current::Finalize();
	OrbitLogger::LogCollector::Stop();
	return Result ? 0 : 1;
}

#endif // _BUILDING_TOOLS_
