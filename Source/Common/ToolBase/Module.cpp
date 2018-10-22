
#include <OrbitLogger/src/OrbitLogger.h>

#include "Module.h"
#include "iSettingsUser.h"    
#include "ModuleRegistration.h"

namespace MoonGlare {

SharedModuleManager ModuleManager::CreateModuleManager() {
	struct ModMgrImpl : public ModuleManager {
		ModMgrImpl() {}
	};
	return std::make_shared<ModMgrImpl>();
}

ModuleManager::ModuleManager() {}

bool ModuleManager::Initialize() {
    RegisterTollBaseAllModules();

	bool ret = true;
	auto self = shared_from_this();
	ModuleClassRgister::GetRegister()->Enumerate([this, &ret, self] (auto &ci) {
		auto ptr = ci.SharedCreate(self);
		m_Modules.emplace_back(ptr);
        if (ptr->GetModuleName().empty()) {
            ptr->SetAlias(ci.Alias);
        }
	});

    size_t modcnt = 0;
  	for (auto &it : m_Modules) {
        if (!it->Initialize()) {
            ret = false;
            AddLogf(Error, "Module init failed: %s", it->GetModuleName().c_str());
        }
        else {
            AddLogf(Info, "Loaded module[%02d]: %s", ++modcnt, it->GetModuleName().c_str());
        }
	}

	for (auto &it : m_Modules) {
		if (!it->PostInit()) {
            ret = false;
            AddLogf(Error, "Module post init failed: %s", it->GetModuleName().c_str());
        }
	}

	LoadSettigs();
	return true;
}

bool ModuleManager::Finalize() {
	SaveSettigs();
	for (auto &it : m_Modules) {
		it->Finalize();
	}
    auto mod = std::move(m_Modules);
	return true;
}

void ModuleManager::LoadSettigs() {
	for (auto &item : QuerryInterfaces<iSettingsUser>()) {
		item.m_Interface->LoadSettings();
	}
}

void ModuleManager::SaveSettigs() {
	for (auto &item : QuerryInterfaces<iSettingsUser>()) {
		item.m_Interface->SaveSettings();
	}
}

}

