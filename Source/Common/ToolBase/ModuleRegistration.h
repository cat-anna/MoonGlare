#pragma once 

namespace MoonGlare {

void RegisterBaseModules();
void RegisterImporterModules();

inline void RegisterAllModules() {
    RegisterBaseModules();
    RegisterAllModules();
}

}
