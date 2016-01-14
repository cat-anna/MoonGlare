#ifndef GameAppH
#define GameAppH

namespace MoonGlare {
namespace Application {

class GameApplication : public iApplication {
	GABI_DECLARE_STATIC_CLASS(GameApplication, cRootClass);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	GameApplication(int argc, char** argv);
	~GameApplication();

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
