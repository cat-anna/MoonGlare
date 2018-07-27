#include "EventInfo.h"

namespace MoonGlare::Component {

EventClassId BaseEventInfo::idAlloc = 0;
//BaseEventInfo::EventClassesTypeTable BaseEventInfo::eventClassesTypeInfo;

BaseEventInfo::EventClassesTypeTable& BaseEventInfo::GetEventClassesTypeInfo() {
    static EventClassesTypeTable table;
    return table;
}

void BaseEventInfo::Dump(std::ostream &output) {
	output << "Event classes:\n";

    for (EventClassId i = 0; i < idAlloc; ++i) {
        auto tinfo = GetEventTypeInfo(i);
        if (!tinfo.infoPtr)
            continue;

        auto & ttype = tinfo.infoPtr->GetTypeInfo();

        std::string flags;
#ifdef DEBUG
        flags += tinfo.pod ? "POD" : "CLASS";
#endif
        if (tinfo.apiInitFunc)
            flags += ",SCRIPTAPI";

        output << fmt::format("{:2}. {:100} size:{:3} flags:{:30} Event:{:30 Handler:{:30}\n",
            i, ttype.name(), tinfo.byteSize, flags, tinfo.EventName, tinfo.HandlerName);
    }
}

}
