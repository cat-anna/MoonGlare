
#pragma once

#include "file_icon_info.hpp"
#include <runtime_modules.h>
#include <string>
#include <unordered_map>

namespace MoonGlare::Tools::RuntineModules {

class BasicIconsProviderModule : public iModule, public iFileIconInfo {
public:
    BasicIconsProviderModule(SharedModuleManager modmgr);
    std::vector<FileIconInfo> GetFileIconInfo() const override;

private:
};

} // namespace MoonGlare::Tools::RuntineModules
