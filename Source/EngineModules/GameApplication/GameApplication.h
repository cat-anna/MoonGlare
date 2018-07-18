#ifndef GameAppH
#define GameAppH

namespace MoonGlare {
namespace Application {

class GameApplication : public iApplication {
	SPACERTTI_DECLARE_STATIC_CLASS(GameApplication, cRootClass);
public:
	GameApplication(int argc, char** argv);
	~GameApplication();

	virtual const char* ExeName() const override;
private:
	int m_argc;
	char** m_argv;
};

} //namespace Application
} //namespace MoonGlare

#endif
