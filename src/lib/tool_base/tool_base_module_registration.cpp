

#include "tool_base_module_registration.h"
#include "importer/importer.h"
#include "runtime_modules/background_process.h"
#include "runtime_modules/remote_console_module.h"
#include "runtime_modules/settings_user.h"

namespace MoonGlare::Tools {

void RegisterBaseModules() {
    using namespace RuntineModules;
    ModuleClassRegister::Register<iSettings> iSettingsReg("iSettings");
    ModuleClassRegister::Register<BackgroundProcessManager> BackgroundProcessManagerReg("BackgroundProcessManager");
    ModuleClassRegister::Register<RemoteConsoleModule> RemoteConsoleReg("RemoteConsoleModule");
}

void RegisterImporterModules() {
    using namespace Importer;
    ModuleClassRegister::Register<ImporterProvider> ImporterProviderReg("ImporterProvider");
}

} // namespace MoonGlare::Tools
