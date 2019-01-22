

#include "Importer/iImporter.h"

#include "Modules/ChangesManager.h"
#include "Modules/iBackgroundProcess.h"
#include "Modules/iSettingsUser.h"

namespace MoonGlare {

void RegisterBaseModules() {
    ModuleClassRegister::Register<iSettings> iSettingsReg("iSettings");
    ModuleClassRegister::Register<ChangesManager> ChangesManagerReg("ChangesManager");
    ModuleClassRegister::Register<BackgroundProcessManager> BackgroundProcessManagerReg("BackgroundProcessManager");
}

void RegisterImporterModules() {
    //Importer::ImporterProvider::RegisterModule();
}

}
