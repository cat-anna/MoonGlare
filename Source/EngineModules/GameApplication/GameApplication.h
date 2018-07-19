#pragma once

#include <Engine/Application.h>

namespace MoonGlare {

class GameApplication : public Application {
public:
	GameApplication(int argc, char** argv);
	~GameApplication();

	virtual const char* ExeName() const override;
private:
	int m_argc;
	char** m_argv;
};

} //namespace MoonGlare
