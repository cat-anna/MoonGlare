

#include "Importer/iImporter.h"
#include "iSettingsUser.h"

namespace MoonGlare {

void RegisterBaseModules() {
    ModuleClassRgister::Register<iSettings> iSettingsReg("iSettings");
}

void RegisterImporterModules() {
    Importer::ImporterProvider::RegisterModule();
}

}
