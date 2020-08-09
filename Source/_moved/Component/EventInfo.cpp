#include "EventInfo.h"

namespace MoonGlare::Component {

/*@ [Scripts/EventsReference] Event reference 
    Some events are accessible through Events module, but most of them
    are only available through proper handler in ScriptObject.
@*/

/*@ [EventsReference/EventsCommonApi] Events common api
    All event objects share following constants:
    * `EventObject.EventName` - Name of the event
    * `EventObject.HandlerName` - Handler name to be called in ScriptObject
    * `EventObject.EventId` - internal event id
@*/

std::underlying_type_t<EventClassId> BaseEventInfo::idAlloc = 0;
//BaseEventInfo::EventClassesTypeTable BaseEventInfo::eventClassesTypeInfo;

BaseEventInfo::EventClassesTypeTable& BaseEventInfo::GetEventClassesTypeInfo() {
    static EventClassesTypeTable table;
    return table;
}

void BaseEventInfo::Dump(std::ostream &output) {
	output << "Event classes:\n";
                                                       
    for (std::underlying_type_t<EventClassId> i = 0; i <= idAlloc; ++i) {
        auto tinfo = GetEventTypeInfo((EventClassId)i);
        if (!tinfo.infoPtr)
            continue;

        auto & ttype = tinfo.infoPtr->GetTypeInfo();

        std::string flags;
#ifdef DEBUG
        flags += tinfo.pod ? "POD" : "CLASS";
#endif
        if (tinfo.apiInitFunc)
            flags += ",SCRIPTAPI";
        if (tinfo.isPublic)
            flags += ",PUBLIC";
        if (tinfo.hasRecipient)
            flags += ",RECIPIENT";

        output << fmt::format("{:2}. {:80} size:{:3} flags:{:50} EventName:{:30} Handler:{:30}\n",
            i, ttype.name(), tinfo.byteSize, flags, tinfo.EventName, tinfo.HandlerName);
    }
}

}
