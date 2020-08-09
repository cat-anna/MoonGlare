#pragma once

namespace MoonGlare::Tools {

void RegisterBaseModules();
void RegisterBaseImporterModules();

inline void RegisterAllBaseModules() {
    RegisterBaseModules();
    RegisterBaseImporterModules();
}

} // namespace MoonGlare::Tools
