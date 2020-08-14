#pragma once

namespace MoonGlare::StarVfs {

class iVfsModuleInterface;

class iVfsModule {
public:
    explicit iVfsModule(iVfsModuleInterface *module_interface) : module_interface(module_interface){};
    virtual ~iVfsModule() = default;

    virtual void Execute();

protected:
    iVfsModuleInterface *const module_interface;
};

} // namespace MoonGlare::StarVfs