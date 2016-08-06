
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/Core/Console.h>
#include <Engine/iApplication.h>
#include <Engine/ModulesManager.h>
#include "GameApplication.h"

namespace MoonGlare {
namespace Application {
 
struct AppModule : public MoonGlare::Modules::ModuleInfo {
	AppModule(): BaseClass("GameApplication", ModuleType::Application) { }
};
DEFINE_MODULE(AppModule);

//----------------------------------------------------------------

SPACERTTI_IMPLEMENT_STATIC_CLASS(GameApplication);

GameApplication::GameApplication(int argc, char** argv) {
	m_argc = argc;
	m_argv = argv;
}

GameApplication::~GameApplication() {
}

const char* GameApplication::ExeName() const {
	return m_argv[0];
}

#define _chk(WHAT, ERRSTR, ...) do { if(!(WHAT)) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _chk_ret(WHAT, ERRSTR, ...) do { if(!(WHAT)) { AddLogf(Error, ERRSTR, __VA_ARGS__); return false; } } while(false)

bool GameApplication::Initialize() {
#define _init_chk(WHAT, ERRSTR, ...) do { if(!(WHAT)->Initialize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); return false; } } while(false)

	using Modules::ModulesManager;
	using MoonGlare::Core::Console;
	using FileSystem::MoonGlareFileSystem;
//	using MoonGlare::Core::EntityManager;
//	using MoonGlare::Core::Component::ComponentManager;
	using MoonGlare::Core::Scene::ScenesManager;
	using ::Core::Scripts::cScriptEngine;

//	_chk_ret(EntityManager::Initialize(),					"Entity manager initialization failed!");
//	_chk_ret(ComponentManager::Initialize(),				"Component manager initialization failed");

	_init_chk(new MoonGlareFileSystem(),					"Unable to initialize internal filesystem!");
	_init_chk(new cScriptEngine(),							"Unable to initialize script engine!");
	_init_chk(new ModulesManager(),							"Unable to initialize modules manager!");
	_init_chk(new MoonGlare::Core::Data::Manager(),			"Unable to initialize data manager!");
	
	Graphic::Window::InitializeWindowSystem();
	new Graphic::cRenderDevice(std::make_unique<Graphic::Window>(true));

	new ScenesManager();
	new MoonGlare::Core::Engine();
		
	if (Settings->Engine.EnableConsole)
		_init_chk(new Console(),							"Unable to initialize console!");
	
	Graphic::GetRenderDevice()->Initialize();
	MoonGlare::Core::GetEngine()->Initialize();
	MoonGlare::Core::GetScenesManager()->Initialize();

	//Temporary solution which probably will be used for eternity
	Graphic::GetRenderDevice()->GetContext()->SetInputProcessor(MoonGlare::Core::GetEngine()->GetWorld()->GetInputProcessor());

	AddLog(Debug, "Application initialized");
#undef _init_chk
	return true;
}

bool GameApplication::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::InstanceExists()) break; if(!WHAT::Instance()->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

	using Modules::ModulesManager;
	using MoonGlare::Core::Console;
//	using MoonGlare::Core::EntityManager;
//	using MoonGlare::Core::Component::ComponentManager;
	using MoonGlare::Core::Scene::ScenesManager;
	using ::Core::Scripts::cScriptEngine;

	_del_chk(Console,								"Console finalization failed");

	_finit_chk(ScenesManager,						"Scenes Manager finalization failed");
	_finit_chk(MoonGlare::Core::Engine,				"Engine finalization failed");
	_finit_chk(Graphic::cRenderDevice,				"Render device finalization failed");
	_finit_chk(MoonGlare::Core::Data::Manager,		"Data Manager finalization failed");
	_finit_chk(ModulesManager,						"Finalization of modules manager failed!");
	
	ModulesManager::DeleteInstance();
	ScenesManager::DeleteInstance();
	MoonGlare::Core::Engine::DeleteInstance();
	MoonGlare::Core::Data::Manager::DeleteInstance();

	Graphic::cRenderDevice::DeleteInstance();

	Graphic::Window::FinalzeWindowSystem();

	_del_chk(cScriptEngine,							"Finalization of script engine failed!");
	_del_chk(FileSystem::MoonGlareFileSystem,		"Finalization of filesystem failed!");

//	_chk(ComponentManager::Finalize(),				"Component manager initialization failed");
//	_chk(EntityManager::Finalize(),					"Entity manager initialization failed!");

	AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
	return true;
}

#ifdef DEBUG_SCRIPTAPI

RegisterApiDerivedClass(GameApplication, &GameApplication::RegisterScriptApi);

void GameApplication::RegisterScriptApi(::ApiInitializer &root) {
	root
	.deriveClass<ThisClass, BaseClass>("cGameApplication")
	.endClass()
	;
}

#endif // DEBUG_SCRIPTAPI

} //namespace Application
} //namespace MoonGlare
