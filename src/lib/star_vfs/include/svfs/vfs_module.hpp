#pragma once

namespace MoonGlare::StarVfs {

class iVfsModuleInterface;

class iVfsModule {
public:
    explicit iVfsModule(iVfsModuleInterface *module_interface) : module_interface(module_interface){};
    virtual ~iVfsModule() = default;

protected:
    iVfsModuleInterface *const module_interface;
};

} // namespace MoonGlare::StarVfs