

#include "Importer/iImporter.h"

#include "Modules/ChangesManager.h"
#include "Modules/RemoteConsoleModule.h"
#include "Modules/iBackgroundProcess.h"
#include "Modules/iSettingsUser.h"

namespace MoonGlare {

void RegisterBaseModules() {
    ModuleClassRegister::Register<Module::iSettings> iSettingsReg("iSettings");
    ModuleClassRegister::Register<Module::ChangesManager> ChangesManagerReg("ChangesManager");
    ModuleClassRegister::Register<Module::BackgroundProcessManager> BackgroundProcessManagerReg("BackgroundProcessManager");
    ModuleClassRegister::Register<Module::RemoteConsole> RemoteConsoleReg("RemoteConsole");
}

void RegisterImporterModules() {
    Importer::ImporterProvider::RegisterModule();
}

}
