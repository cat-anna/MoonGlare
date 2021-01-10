#include "basic_icons_provider_module.hpp"
#include "qt_gui_icons.h"

namespace MoonGlare::Tools::RuntineModules {

BasicIconsProviderModule::BasicIconsProviderModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {
}

std::vector<BasicIconsProviderModule::FileIconInfo> BasicIconsProviderModule::GetFileIconInfo() const {
    return std::vector<FileIconInfo>{
        FileIconInfo{".blend", ICON_16_3DMODEL_RESOURCE}, //
        FileIconInfo{".3ds", ICON_16_3DMODEL_RESOURCE},   //
        FileIconInfo{".fbx", ICON_16_3DMODEL_RESOURCE},   //
    };
}

} // namespace MoonGlare::Tools::RuntineModules
