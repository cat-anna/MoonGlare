

#include "tool_base_module_registration.h"
#include "importer/importer.h"
#include "runtime_modules/application_settings.hpp"
#include "runtime_modules/background_process.h"
#include "runtime_modules/component_register_module.hpp"
#include "runtime_modules/editable_type_provider_module.hpp"
#include "runtime_modules/remote_console_module.hpp"

namespace MoonGlare::Tools {

void RegisterBaseModules() {
    using namespace RuntineModules;
    ModuleClassRegister::Register<ApplicationSettings> iSettingsReg("ApplicationSettings");
    ModuleClassRegister::Register<BackgroundProcessManager> BackgroundProcessManagerReg(
        "BackgroundProcessManager");
    ModuleClassRegister::Register<RemoteConsoleModule> RemoteConsoleReg("RemoteConsoleModule");
    ModuleClassRegister::Register<EditableTypeProviderModule> EditableTypeProviderReg(
        "EditableTypeProviderModule");
    ModuleClassRegister::Register<ComponentRegisterModule> ComponentRegisterReg(
        "ComponentRegisterModule");
}

void RegisterImporterModules() {
    using namespace Importer;
    ModuleClassRegister::Register<ImporterProvider> ImporterProviderReg("ImporterProvider");
}

} // namespace MoonGlare::Tools
