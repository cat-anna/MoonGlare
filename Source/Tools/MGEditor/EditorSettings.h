#pragma once

#include <EditorSettings.x2c.h>
#include <ToolBase/Module.h>
#include <ToolBase/Modules/iSettingsUser.h>

namespace MoonGlare {
namespace Editor {

class EditorSettings : public iModule, public iSettings::CustomConfigSet {
public:
	EditorSettings(SharedModuleManager modmgr);
	~EditorSettings();

	x2c::Settings::EditorState_t& GetState() { return m_State; }
	x2c::Settings::EditorConfiguration_t& GetConfiguration() { return m_Configuration; }

    virtual void SaveSettings(pugi::xml_node node) override;
    virtual void LoadSettings(pugi::xml_node node) override;
    virtual std::string GetName() const override;
protected:
	x2c::Settings::EditorState_t m_State;
	x2c::Settings::EditorConfiguration_t m_Configuration;
};

} //namespace Editor
} //namespace MoonGlare

