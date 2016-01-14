#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iApplication.h>
#include "GameApplication.h"

#include <GabiLib/src/utils/ParamParser.cpp>
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

const GabiLib::ProgramParameters::Parameter Parameters[] = {
//	{'s', 1, 0, 			SettingsFile,	"Select settings file."},
// 	{'', 0, f, 0, ""},

	//{'m', 0, 0, flag_disableMouseUnhook, "Disable mouse unhook", 0},
 	//{'v', 0, 0, flag_HideConsole, "Hide console", 0},
 	{'d', 0, 0, flag_DisableConsole, "Disable console", 0},

 	//{'f', 1, 0, option_setFPS, "Set desired FPS", 0},
 	{'x', 1, 0, option_setWidth, "Set window width", 0},
 	{'y', 1, 0, option_setHeight, "Set window height", 0},

	{'\0', 0, 0, 0, 0, 0},
}; 

GabiLib::ProgramParameters Params = {
	GabiLib::ProgramParameters::disable_helpmsg_on_noparams,
	"FPS Engine",
	Settings->Modules,
	"MainModule OtherModules",
	Parameters,
	0,
	0,
}; 

#ifndef _BUILDING_TOOLS_
int main(int argc, char** argv) {
	int Result = 0;
#pragma warning ( suppress: 4244 )
	srand(time(NULL));
	::Thread::SetInfo("MAIN", true);

	auto sink = std::make_unique<Log::StdFileLoggerSink>();
	sink->open("logs/Engine.log");
	sink->Enable();
	auto sink2 = std::make_unique<Log::StdNoDebugFileLoggerSink>();
	sink2->open("logs/Engine.filtered.log");
	sink2->Enable();
	new ::Log::LogEngine();

	Config::Current::Initialize();
	AddLog(Thread, "MainThread");
	Settings->Load();
	//Core::Settings->DisableSave();
	do {
		bool doRestart = false;
		try {
			Params.Parse(argc, argv);
			new GameApplication(argc, argv);
#ifdef DEBUG
			if (Settings->Modules.empty()) {
				Settings->Modules.push_back("./Source/datamodules/internal");
				Settings->Modules.push_back("./Source/datamodules/debug");
				Settings->Modules.push_back("./modules/MazeGame");
			}
#endif
			EnableScriptsInThisThread();
			if (!GetApplication()->Initialize()) {
				AddLog(Error, "Unable to initialize application!");
				Result = 1;
			} 
		
			if (Result == 0) //when there are no errors so far
				::Core::GetEngine()->EngineMain();

			if (!GetApplication()->Finalize()) {
				AddLog(Error, "Unable to finalize application!");
				Result = 2;
			}
			doRestart = GetApplication()->IsDoRestart();
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
		if (Result == 0)
			Config::Current::CheckInstances();
#endif
		if (!doRestart)
			break;
		AddLog(Debug, "Performing application restart");
	} while (true);

	Settings->Save();
	Config::Current::Finalize();
	sink.reset();
	sink2.reset();
	Log::LogEngine::DeleteInstance();
	return Result;
}
#endif // _BUILDING_TOOLS_
