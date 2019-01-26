
#include <OrbitLogger/src/OrbitLogger.h>

#include "AppConfig.h"    
#include "Module.h"
#include "Modules/iSettingsUser.h"    

namespace MoonGlare {

struct AppConfigImpl : public AppConfig {
    AppConfigImpl(SharedModuleManager modmgr, MapType appConfig) :AppConfig(modmgr){
        values.swap(appConfig);
        SetAlias("AppConfig");
    }
};

struct ModMgrImpl : public ModuleManager {
    ModMgrImpl() { }

    void SetAppConfig(AppConfig::MapType appConfig) {
        m_Modules.emplace_back(std::make_shared<AppConfigImpl>(shared_from_this(), std::move(appConfig)));
    }
};

SharedModuleManager ModuleManager::CreateModuleManager(AppConfig::MapType AppConfig) {
	auto modmgr = std::make_shared<ModMgrImpl>();
    modmgr->SetAppConfig(std::move(AppConfig));
    return modmgr;
}

ModuleManager::ModuleManager() {}

bool ModuleManager::Initialize() {
	bool ret = true;
	auto self = shared_from_this();

    m_Modules.reserve(ModuleClassRegister::GetRegister()->GetCount());
    ModuleClassRegister::GetRegister()->Enumerate([this, &ret, self] (auto &ci) {
		auto ptr = ci.SharedCreate(self);
		m_Modules.emplace_back(ptr);
        if (ptr->GetModuleName().empty()) {
            ptr->SetAlias(ci.Alias);
        }
	});
    m_Modules.shrink_to_fit();

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
	for (auto &item : QuerryInterfaces<Module::iSettingsUser>()) {
		item.m_Interface->LoadSettings();
	}
}

void ModuleManager::SaveSettigs() {
	for (auto &item : QuerryInterfaces<Module::iSettingsUser>()) {
		item.m_Interface->SaveSettings();
	}
}

}


