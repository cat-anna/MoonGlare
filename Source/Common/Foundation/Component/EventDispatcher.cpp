#include "EventDispatcher.h"

namespace MoonGlare::Component {
	
EventDispatcher::EventDispatcher() {
	static bool loginit = false;
	if (!loginit) {
        //TODO: this is ugly
		loginit = true;
		::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Event, "EVNT");
	}
    buffer.Zero();
}

void EventDispatcher::SetEventSink(lua_State *lua, EventScriptSink *sink) {
    assert((lua && sink) || (!lua && !sink));
    luaState = lua;
    eventSink = sink;
}

void EventDispatcher::Step() {
    if (buffer.Empty())
        return;

    std::lock_guard<std::recursive_mutex> lock(bufferMutex);

    for (auto item : buffer) {
        const BaseQueuedEvent *base = reinterpret_cast<const BaseQueuedEvent*>(item.memory);
        (this->*(base->sendFunc))(*base);
    }
    buffer.Clear();
}

}
