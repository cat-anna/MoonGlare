
#include "runtime_modules.h"
#include "runtime_modules/app_config.h"
#include "runtime_modules/application_settings.hpp"
#include <orbit_logger.h>

namespace MoonGlare::Tools {

using RuntineModules::AppConfig;

struct AppConfigImpl : public AppConfig {
    AppConfigImpl(SharedModuleManager modmgr, MapType appConfig) : AppConfig(modmgr) {
        values.swap(appConfig);
        SetAlias("AppConfig");
    }
};

struct ModMgrImpl : public ModuleManager {
    ModMgrImpl() {}

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

    ModuleClassRegister::GetRegister()->Enumerate([this, &ret, self](auto &ci) {
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
        } else {
            AddLogf(Info, "Loaded module[%02d]: %s", ++modcnt, it->GetModuleName().c_str());
        }
    }

    for (auto &it : m_Modules) {
        if (!it->PostInit()) {
            ret = false;
            AddLogf(Error, "Module post init failed: %s", it->GetModuleName().c_str());
        }
    }

    try {
        QueryModule<RuntineModules::ApplicationSettings>()->Load();
    } catch (const std::exception &e) {
        AddLogf(Error, "Failed to load settings: %s", e.what());
    }

    return true;
}

bool ModuleManager::Finalize() {
    try {
        QueryModule<RuntineModules::ApplicationSettings>()->Save();
    } catch (const std::exception &e) {
        AddLogf(Error, "Failed to save settings: %s", e.what());
    }

    for (auto &it : m_Modules) {
        it->Finalize();
    }
    auto mod = std::move(m_Modules);
    return true;
}

} // namespace MoonGlare::Tools
