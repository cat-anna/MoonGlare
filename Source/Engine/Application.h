#pragma once

#include <Renderer/iRendererFacade.h>
#include <Foundation/Settings.h>

namespace MoonGlare::x2c::Settings {
struct EngineSettings_t;
}

namespace MoonGlare {

class Application {
public:
    Application();
    virtual ~Application();

    void SaveSettings();
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
    virtual std::string ApplicationPath() const = 0;
    virtual std::string SettingsPath() const;

    bool IsActive() const { return m_Flags.m_Active; }
    bool DoRestart() const { return m_Flags.m_Restart; }
    void SetRestart(bool v) { m_Flags.m_Restart = v; }
protected:
    union Flags {
        struct {
            bool m_Active : 1;
            bool m_Restart : 1;
        };
        uint32_t m_UintValue;
    };
    Flags m_Flags;
    std::unique_ptr<World> m_World;

    Renderer::UniqueRenderer m_Renderer;
    std::shared_ptr<Settings> settings;

    void LoadSettings();
    void InitLogger();

    void WaitForFirstScene();

    Renderer::ContextCreationInfo GetDisplaySettings();

    virtual std::shared_ptr<Settings> GetUpperLayerSettings() { return nullptr; };
};

} //namespace MoonGlare
