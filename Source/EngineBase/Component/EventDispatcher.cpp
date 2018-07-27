#include "EventDispatcher.h"

namespace MoonGlare::Component {
	
EventDispatcher::EventDispatcher() {
	static bool loginit = false;
	if (!loginit) {
        //TODO: this is ugly
		loginit = true;
		::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Event, "EVNT");
	}
}

void EventDispatcher::SetEventSink(lua_State *lua, EventScriptSink *sink) {
    assert((lua && sink) || (!lua && !sink));
    luaState = lua;
    eventSink = sink;
}

}
