

#include "Importer/iImporter.h"
#include "iSettingsUser.h"

#include "Modules/ChangesManager.h"

namespace MoonGlare {

void RegisterBaseModules() {
    ModuleClassRgister::Register<iSettings> iSettingsReg("iSettings");
    ModuleClassRgister::Register<ChangesManager> ChangesManagerReg("ChangesManager");
}

void RegisterImporterModules() {
    Importer::ImporterProvider::RegisterModule();
}

}
