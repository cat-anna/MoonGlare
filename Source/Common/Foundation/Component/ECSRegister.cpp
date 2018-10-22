#include "ECSRegister.h"

namespace MoonGlare::Component {

void ECSRegister::Dump() {
    using DumpFunc = void(*)(std::ostream &out);

    DumpFunc DumpFuncArr[] = {
        &BaseComponentInfo::Dump,
        &BaseEventInfo::Dump,
        &BaseSystemInfo::Dump,
    };

    for (auto func : DumpFuncArr) {
        std::stringstream ss;
        ss << "\n";
        func(ss);
        AddLog(Debug, ss.str());
    }
}

}