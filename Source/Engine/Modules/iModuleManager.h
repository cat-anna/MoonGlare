#pragma once

namespace MoonGlare::Modules {

class iModuleManager {
public:
    virtual ~iModuleManager() {}

    virtual void LoadSettings(const pugi::xml_node node) = 0;
    virtual void SaveSettings(pugi::xml_node node) const = 0;
};

}