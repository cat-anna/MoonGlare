#include "changes_manager.h"

namespace MoonGlare::Tools::RuntineModules {

ChangesManager::ChangesManager(SharedModuleManager modmgr) : iModule(modmgr) {}

ChangesManager::~ChangesManager() {}

void ChangesManager::SetModifiedState(iChangeContainer *sender, bool value) {
    if (value) {
        m_State[sender].m_Modified = value;
    } else {
        m_State.erase(sender);
    }
    emit Changed(sender, value);
}

void ChangesManager::SaveAll() {
    while (!m_State.empty()) {
        auto it = m_State.begin()->first;
        it->SaveChanges();
    }
}

} // namespace MoonGlare::Tools::RuntineModules
