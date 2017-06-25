/*
  * Generated by cppsrc.sh
  * On 2016-10-09 20:20:47,90
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include "iCustomEnum.h"

namespace MoonGlare {
namespace QtShared {

ModuleClassRgister::Register<CustomEnumProvider> CustomEnumProviderReg("CustomEnumProvider");

CustomEnumProvider::CustomEnumProvider(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

bool CustomEnumProvider::PostInit() {
    for (auto module : GetModuleManager()->QuerryInterfaces<iCustomEnumSupplier>()) {
        for (auto e : module.m_Interface->GetCustomEnums()) {
            enumMap[ToLower(e->GetEnumTypeName())] = e;
            AddLogf(Info, "Got CustomEnum provider: %s", e->GetEnumTypeName().c_str());
        }
    }
    return true;
}

const std::shared_ptr<iCustomEnum> CustomEnumProvider::GetEnum(const std::string &Typename) const {
    auto it = enumMap.find(ToLower(Typename));
    if (it == enumMap.end())
        return nullptr;
    return it->second.lock();
}

} //namespace QtShared 
} //namespace MoonGlare 
