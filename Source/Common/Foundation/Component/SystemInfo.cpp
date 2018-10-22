#include <fmt/format.h>

#include "SystemInfo.h"

namespace MoonGlare::Component {

std::underlying_type_t<SystemClassId> BaseSystemInfo::idAlloc = 0;   

BaseSystemInfo::SystemClassesTypeTable& BaseSystemInfo::GetSystemClassesTypeInfo() {
    static BaseSystemInfo::SystemClassesTypeTable SystemClassesTypeInfo;
    return SystemClassesTypeInfo;
}

void BaseSystemInfo::Dump(std::ostream &output) {
    output << "System classes:\n";
    for (std::underlying_type_t<SystemClassId> i = 0; i < (size_t)idAlloc; ++i) {
        auto tinfo = GetSystemTypeInfo((SystemClassId)i);
        if (!tinfo.infoPtr)
            continue;
        auto & ttype = tinfo.infoPtr->GetTypeInfo();

        std::string flags;

        //if (tinfo.required)
            //flags += "REQUIRED";

        output << fmt::format("{:2}. {:100} name:{:20} flags:{:50}\n", 
            i, ttype.name(), tinfo.systemName, flags);
    }
}

std::unordered_map<std::string, SystemClassId> *gSystemNameMapping = nullptr;

void BaseSystemInfo::SetNameMapping(SystemClassId ccid, std::string name) {
    if (!gSystemNameMapping)
        gSystemNameMapping = new std::unordered_map<std::string, SystemClassId>();
    assert(gSystemNameMapping->find(name) == gSystemNameMapping->end());
    gSystemNameMapping->emplace(std::move(name), ccid);
}

std::optional<SystemClassId> BaseSystemInfo::GetClassByName(const std::string &cname) {
    if (!gSystemNameMapping)
        return std::nullopt;
    auto it = gSystemNameMapping->find(cname);
    if (it == gSystemNameMapping->end())
        return std::nullopt;
    return it->second;
}

} //namespace MoonGlare::Core
