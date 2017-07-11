#pragma once

namespace MoonGlare::Modules {

class iModule : public std::enable_shared_from_this<iModule> {
public:
    iModule(World *world) : world(world) {}
    virtual ~iModule() {}

    virtual std::string GetName() const { return ""; };

    virtual void LoadSettings(const pugi::xml_node node) { }
    virtual void SaveSettings(pugi::xml_node node) const { }     

    virtual void OnPostInit() {}
protected:
    World *GetWorld() const { return world; }
private:
    World *world = nullptr;
};

using ModuleClassRegister = ::Space::DynamicClassRegister<iModule, World*>;

}
                                           