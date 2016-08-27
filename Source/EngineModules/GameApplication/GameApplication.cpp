
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/iApplication.h>
#include <Engine/Core/DataManager.h>
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

bool GameApplication::PostSystemInit() {
	GetDataMgr()->LoadGlobalData();
	Core::GetScenesManager()->SetNextScene(GetDataMgr()->GetConfiguration().m_FirstScene);
	return true;
}

const char* GameApplication::ExeName() const {
	return m_argv[0];
}

} //namespace Application
} //namespace MoonGlare
