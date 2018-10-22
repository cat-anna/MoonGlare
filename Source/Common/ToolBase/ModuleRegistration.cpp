
#include "Importer/iImporter.h"

namespace MoonGlare {

void RegisterTollBaseAllModules() {
    Importer::ImporterProvider::RegisterModule();
}

}
