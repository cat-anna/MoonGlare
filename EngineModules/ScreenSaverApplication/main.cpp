#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iApplication.h>
#include "ScreenSaverApplication.h"

#include <GabiLib/src/utils/ParamParser.cpp>
using namespace std;
using namespace MoonGlare::Application;

const GabiLib::ProgramParameters::Parameter Parameters[] = {
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

int main(int argc, char** argv) {
	int Result = 0;
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
			new ScreenSaverApplication(argc, argv);
//#ifdef DEBUG
			if (Settings->Modules.empty()) {
				Settings->Modules.push_back("./Source/datamodules/internal");
				Settings->Modules.push_back("./Source/datamodules/debug");
				Settings->Modules.push_back("./modules/Primary");
			}
//#endif
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
