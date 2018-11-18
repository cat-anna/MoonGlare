#pragma once

#include <Foundation/InterfaceMap.h>

#include <Core/Scene/iSceneManager.h>

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

    Core::Engine* GetEngine();

    void SetRendererFacade(Renderer::RendererFacade *c);
	Renderer::RendererFacade* GetRendererFacade() { return m_RendererFacade; }
    void SetScriptEngine(Core::Scripts::ScriptEngine *se) { m_ScriptEngine = se; }
private:
	std::unique_ptr<Core::InputProcessor> m_InputProcessor;
	std::unique_ptr<Core::Scene::iSceneManager> m_ScenesManager;
    std::unique_ptr<Core::RuntimeConfiguration> runtimeConfiguration;
    std::unique_ptr<Component::EntityManager> entityManager;

	//not owning
	Renderer::RendererFacade *m_RendererFacade = nullptr;
	Core::Scripts::ScriptEngine *m_ScriptEngine = nullptr;
    Component::EventDispatcher *eventDispatcher = nullptr;
};

} //namespace MoonGlare
