#pragma once

#include <Renderer/iRendererFacade.h>

namespace MoonGlare::x2c::Settings {
struct EngineSettings_t;
}

namespace MoonGlare {

class Application {
public:
    Application();
    virtual ~Application();

    void LoadSettings();
    void SaveSettings();
    void SettingsChanged() { m_Flags.m_SettingsChanged = true; }
    void Restart();

    virtual void Initialize();
    virtual void Execute();
    virtual void Finalize();

    virtual void LoadDataModules();

    virtual bool PreSystemInit();
    virtual bool PostSystemInit();

    virtual void Exit();
    virtual void OnActivate();
    virtual void OnDeactivate();
    virtual const char* ExeName() const;

    bool IsActive() const { return m_Flags.m_Active; }
    bool DoRestart() const { return m_Flags.m_Restart; }
    void SetRestart(bool v) { m_Flags.m_Restart = v; }
protected:
    union Flags {
        struct {
            bool m_Initialized : 1;
            bool m_SettingsLoaded : 1;
            bool m_SettingsChanged : 1;
            bool m_Active : 1;
            bool m_Restart : 1;
        };
        uint32_t m_UintValue;
    };
    Flags m_Flags;
    std::unique_ptr<World> m_World;

    Renderer::UniqueRenderer m_Renderer;

    std::string m_ConfigurationFileName;
    std::string m_SettingsFileName;
    std::unique_ptr<x2c::Settings::EngineSettings_t> m_Configuration;

    void WaitForFirstScene();

    Renderer::ContextCreationInfo GetDisplaySettings();
};

} //namespace MoonGlare
