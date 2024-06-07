#include "file_icon_provider_module.hpp"
#include <boost/algorithm/string/case_conv.hpp>

namespace MoonGlare::Tools::RuntineModules {

FileIconProviderModule::FileIconProviderModule(SharedModuleManager modmgr)
    : iModule(std::move(modmgr)) {
}

bool FileIconProviderModule::PostInit() {
    for (auto module : GetModuleManager()->QueryInterfaces<iFileIconInfo>()) {
        for (auto &item : module.interface->GetFileIconInfo()) {
            file_icon_map[boost::to_lower_copy(item.ext)] = item.icon;
            AddLogf(Info, "Associated icon: %s->%s", item.ext.c_str(),
                    module.module->GetModuleName().c_str());
        }
    }
    return true;
}

const std::string &FileIconProviderModule::GetExtensionIcon(const std::string &ext) const {
    return file_icon_map.at(boost::to_lower_copy(ext));
}

const std::string &FileIconProviderModule::GetExtensionIcon(const std::string &ext,
                                                            const std::string &default_icon) const {
    auto it = file_icon_map.find(boost::to_lower_copy(ext));
    if (it == file_icon_map.end())
        return default_icon;
    return it->second;
}

} // namespace MoonGlare::Tools::RuntineModules
