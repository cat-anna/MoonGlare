#ifndef GameAppH
#define GameAppH

namespace MoonGlare {
namespace Application {

class ScreenSaverApplication : public iApplication {
	GABI_DECLARE_STATIC_CLASS(ScreenSaverApplication, cRootClass);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ScreenSaverApplication(int argc, char** argv);
	~ScreenSaverApplication();

	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual const char* ExeName() const override;

	static void RegisterScriptApi(::ApiInitializer &api);
private:
	int m_argc;
	char** m_argv;
};

} //namespace Application
} //namespace MoonGlare

#endif
