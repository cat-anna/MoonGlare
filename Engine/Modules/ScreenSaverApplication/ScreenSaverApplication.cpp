
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/Core/Console.h>
#include <Engine/iApplication.h>
#include <Engine/ModulesManager.h>
#include "ScreenSaverApplication.h"

namespace MoonGlare {
namespace Application {

struct ScreenSaverApplicationModule : public MoonGlare::Modules::ModuleInfo {
	ScreenSaverApplicationModule(): BaseClass("ScreenSaverApplication", ModuleType::Application) { }
};
DEFINE_MODULE(ScreenSaverApplicationModule);

//----------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(ScreenSaverApplication);

ScreenSaverApplication::ScreenSaverApplication(int argc, char** argv) {
	m_argc = argc;
	m_argv = argv;
}

ScreenSaverApplication::~ScreenSaverApplication() {
}

const char* ScreenSaverApplication::ExeName() const {
	return m_argv[0];
}

bool ScreenSaverApplication::Initialize() {
#define _init_chk(WHAT, ERRSTR, ...) do { if(!(WHAT)->Initialize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); return false; } } while(false)

	using Modules::ModulesManager;
	using MoonGlare::Core::Console;
	using FileSystem::MoonGlareFileSystem;
	using ::Core::Scene::ScenesManager;
	using ::Core::Scripts::cScriptEngine;

	::Core::Interfaces::Initialize();

	_init_chk(new MoonGlareFileSystem(),					"Unable to initialize internal filesystem!");
	_init_chk(new cScriptEngine(),							"Unable to initialize script engine!");
	_init_chk(new ModulesManager(),							"Unable to initialize modules manager!");
	_init_chk(new MoonGlare::Core::Data::Manager(),			"Unable to initialize data manager!");
	
	Graphic::Window::InitializeWindowSystem();
	new Graphic::cRenderDevice(std::make_unique<Graphic::Window>(true));

	new ScenesManager();
	new ::Core::Engine();
		
	if (Settings->Engine.EnableConsole)
		_init_chk(new Console(),								"Unable to initialize console!");
	
	Graphic::GetRenderDevice()->Initialize();
	::Core::GetEngine()->Initialize();
	::Core::GetScenesManager()->Initialize();

	AddLog(Debug, "Application initialized");
#undef _init_chk
	return true;
}

bool ScreenSaverApplication::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::InstanceExists()) break; if(!WHAT::Instance()->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); return false; } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

	using Modules::ModulesManager;
	using MoonGlare::Core::Console;
	using ::Core::Scene::ScenesManager;
	using ::Core::Scripts::cScriptEngine;

	_del_chk(Console,								"Console finalization failed");

	_finit_chk(ScenesManager,						"Scenes Manager finalization failed");
	_finit_chk(::Core::Engine,						"Engine finalization failed");
	_finit_chk(Graphic::cRenderDevice,				"Render device finalization failed");
	_finit_chk(MoonGlare::Core::Data::Manager,		"Data Manager finalization failed");
	_finit_chk(ModulesManager,						"Finalization of modules manager failed!");
	
	ModulesManager::DeleteInstance();
	ScenesManager::DeleteInstance();
	::Core::Engine::DeleteInstance();
	MoonGlare::Core::Data::Manager::DeleteInstance();
	Graphic::cRenderDevice::DeleteInstance();

	Graphic::Window::FinalzeWindowSystem();

	_del_chk(cScriptEngine,							"Finalization of script engine failed!");
	_del_chk(FileSystem::MoonGlareFileSystem,		"Finalization of filesystem failed!");

	AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
	return true;
}

#ifdef DEBUG_SCRIPTAPI

RegisterApiDerivedClass(ScreenSaverApplication, &ScreenSaverApplication::RegisterScriptApi);

void ScreenSaverApplication::RegisterScriptApi(::ApiInitializer &root) {
	root
	.deriveClass<ThisClass, BaseClass>("cScreenSaverApplication")
	.endClass()
	;
}

#endif // DEBUG_SCRIPTAPI

} //namespace Application
} //namespace MoonGlare
