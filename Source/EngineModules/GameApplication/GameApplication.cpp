
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Modules/ModuleManager.h>
#include "GameApplication.h"

namespace MoonGlare {
 
GameApplication::GameApplication(int argc, char** argv) {
	m_argc = argc;
	m_argv = argv;
}

GameApplication::~GameApplication() {
}

const char* GameApplication::ExeName() const {
	return m_argv[0];
}

} //namespace MoonGlare
