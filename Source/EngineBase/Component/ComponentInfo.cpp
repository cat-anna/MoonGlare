#include <fmt/format.h>

#include "ComponentInfo.h"

namespace MoonGlare::Component {

ComponentClassId BaseComponentInfo::idAlloc = 0;
BaseComponentInfo::ComponentClassesTypeTable BaseComponentInfo::componentClassesTypeInfo;

void BaseComponentInfo::Dump(std::ostream &output) {
    output << "Component classes:\n";
    for (ComponentClassId i = 0; i < idAlloc; ++i) {
        auto tinfo = GetComponentTypeInfo(i);
        if (!tinfo.infoPtr)
            continue;
        auto & ttype = tinfo.infoPtr->GetTypeInfo();

        std::string flags;
#ifdef DEBUG
        flags += tinfo.pod ? "POD " : "CLASS ";
#endif
        output << fmt::format("{:2}. {:100} size:{:3} flags:{}\n", 
            i, ttype.name(), tinfo.byteSize, flags);
    }
}

} //namespace MoonGlare::Core
