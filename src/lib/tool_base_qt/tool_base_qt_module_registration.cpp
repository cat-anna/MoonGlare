

#include "tool_base_qt_module_registration.h"
#include "runtime_modules/changes_manager.h"

namespace MoonGlare::Tools {

void RegisterBaseQtModules() {
    using namespace RuntineModules;
    ModuleClassRegister::Register<ChangesManager> ChangesManagerReg("ChangesManager");
}

void RegisterBaseQtImporterModules() {}

} // namespace MoonGlare::Tools
