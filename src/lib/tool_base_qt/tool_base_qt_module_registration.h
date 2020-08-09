#pragma once

namespace MoonGlare::Tools {

void RegisterBaseQtModules();
void RegisterBaseQtImporterModules();

inline void RegisterAllBaseQtModules() {
    RegisterBaseQtModules();
    RegisterBaseQtImporterModules();
}

} // namespace MoonGlare::Tools
