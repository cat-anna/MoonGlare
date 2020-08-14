#pragma once

namespace MoonGlare::StarVfs {

class iVfsModuleInterface;

class iVfsExporter {
public:
    explicit iVfsExporter(iVfsModuleInterface *module_interface) : module_interface(module_interface){};
    virtual ~iVfsExporter() = default;

    virtual void StartExport() = 0;

protected:
    iVfsModuleInterface *const module_interface;
};

} // namespace MoonGlare::StarVfs