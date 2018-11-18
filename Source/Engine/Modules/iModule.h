#pragma once

namespace MoonGlare::Modules {

class iModule : public std::enable_shared_from_this<iModule> {
public:
    iModule(InterfaceMap &ifaceMap) : interfaceMap(ifaceMap) { }
    virtual ~iModule() {}

    virtual std::string GetName() const { return ""; };

    virtual void LoadSettings(const pugi::xml_node node) { }
    virtual void SaveSettings(pugi::xml_node node) const { }     

    virtual void OnPostInit() {}
protected:
    InterfaceMap &interfaceMap;
};

using ModuleClassRegister = ::Space::DynamicClassRegister<iModule, InterfaceMap&>;

}
                                           