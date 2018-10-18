#include <fmt/format.h>

#include "ComponentInfo.h"

namespace MoonGlare::Component {

std::underlying_type_t<ComponentClassId> BaseComponentInfo::idAlloc = 0;   

BaseComponentInfo::ComponentClassesTypeTable& BaseComponentInfo::GetComponentClassesTypeInfo() {
    static BaseComponentInfo::ComponentClassesTypeTable componentClassesTypeInfo;
    return componentClassesTypeInfo;
}

void BaseComponentInfo::Dump(std::ostream &output) {
    output << "Component classes:\n";
    for (std::underlying_type_t<ComponentClassId> i = 0; i < (size_t)idAlloc; ++i) {
        auto tinfo = GetComponentTypeInfo((ComponentClassId)i);
        if (!tinfo.infoPtr)
            continue;
        auto & ttype = tinfo.infoPtr->GetTypeInfo();

        std::string flags;
        flags += tinfo.isTrivial ? "TRIVIAL" : "CLASS";

        if (tinfo.scriptPush)
            flags += ",SCRIPT";

        output << fmt::format("{:2}. {:100} size:{:3} flags:{:50}\n", 
            i, ttype.name(), tinfo.byteSize, flags);
    }
}

std::unordered_map<std::string, ComponentClassId> *gComponentNameMapping = nullptr;

void BaseComponentInfo::SetNameMapping(ComponentClassId ccid, std::string name) {
    if (!gComponentNameMapping)
        gComponentNameMapping = new std::unordered_map<std::string, ComponentClassId>();
    assert(gComponentNameMapping->find(name) == gComponentNameMapping->end());
    gComponentNameMapping->emplace(std::move(name), ccid);
}

std::optional<ComponentClassId> BaseComponentInfo::GetClassByName(const std::string &cname) {
    if (!gComponentNameMapping)
        gComponentNameMapping = new std::unordered_map<std::string, ComponentClassId>();
    auto it = gComponentNameMapping->find(cname);
    if (it == gComponentNameMapping->end())
        return std::nullopt;
    return it->second;
}

} //namespace MoonGlare::Core
