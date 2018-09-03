#pragma once

#include <Foundation/InterfaceMap.h>

#include <Core/Scene/iSceneManager.h>

namespace MoonGlare::Resources { class StringTables; }
namespace MoonGlare::Core::Scripts { class ScriptEngine; }

namespace MoonGlare {

class iConsole;
class HandleTable;

class World final : public InterfaceMap {
public:
 	World();
 	~World();

	bool Initialize();
	bool Finalize();
	void PostSystemInit();
	bool PreSystemStart();
	bool PreSystemShutdown();

	bool Step(const Core::MoveConfig &config);

	Component::EntityManager& GetEntityManager() { return *entityManager; }
	Core::InputProcessor* GetInputProcessor() { return m_InputProcessor.get(); }
    Core::RuntimeConfiguration* GetRuntimeConfiguration() { return runtimeConfiguration.get(); }

	Core::Scripts::ScriptEngine *GetScriptEngine() { return m_ScriptEngine; };
	Core::Scene::iSceneManager *GetScenesManager() { return m_ScenesManager.get(); };

    iConsole* GetConsole() { return m_Console; }
    Resources::StringTables* GetStringTables() { return stringTables; }
    Core::Engine* GetEngine();

    void SetRendererFacade(Renderer::RendererFacade *c);
	Renderer::RendererFacade* GetRendererFacade() { return m_RendererFacade; }
    void SetScriptEngine(Core::Scripts::ScriptEngine *se) { m_ScriptEngine = se; }

    void SetConsole(iConsole *c) { m_Console = c; }
    void SetStringTables(Resources::StringTables *st) { stringTables = st; }
private:
	std::unique_ptr<Core::InputProcessor> m_InputProcessor;
	std::unique_ptr<Core::Scene::iSceneManager> m_ScenesManager;
    std::unique_ptr<Core::RuntimeConfiguration> runtimeConfiguration;

    std::unique_ptr<Component::EntityManager> entityManager;
    std::unique_ptr<HandleTable> handleTable;

	//not owning
    iConsole *m_Console = nullptr;
	Renderer::RendererFacade *m_RendererFacade = nullptr;
	Core::Scripts::ScriptEngine *m_ScriptEngine = nullptr;
    Resources::StringTables *stringTables = nullptr;          
    Component::EventDispatcher *eventDispatcher = nullptr;

};

} //namespace MoonGlare
