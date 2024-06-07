
#pragma once

#include "file_icon_info.hpp"
#include <runtime_modules.h>
#include <string>
#include <unordered_map>

namespace MoonGlare::Tools::RuntineModules {

class FileIconProviderModule : public iModule, public iFileIconProvider {
public:
    FileIconProviderModule(SharedModuleManager modmgr);
    bool PostInit() override;

    const std::string &GetExtensionIcon(const std::string &ext) const override;
    const std::string &GetExtensionIcon(const std::string &ext, const std::string &default_icon) const override;

private:
    std::unordered_map<std::string, std::string> file_icon_map;
};

} // namespace MoonGlare::Tools::RuntineModules
