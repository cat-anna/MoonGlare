#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Component/EventInfo.h>

namespace MoonGlare::SoundSystem::Component {

//TODO: StreamStartedEvent 

/*@ [ScriptObjectEventHandler/_] ScriptObject::OnSoundStreamFinishedEvent(soundStreamFinishedEventData)
    This method is called when entity receives this event. See definition of this event for details.
@*/

/*@ [EventsReference/SoundStreamFinishedEvent] SoundStreamFinishedEvent
    This event is send to entity when SoundSourceComponent finishes [CHECK: or loops] playback.
@*/
struct SoundStreamFinishedEvent {
    static constexpr char* EventName = "SoundStreamFinished";
    static constexpr char* HandlerName = "OnSoundStreamFinishedEvent";
    static constexpr bool Public = false;

/*@ [SoundStreamFinishedEvent/_] `SoundStreamFinishedEvent.sender`
    TODO @*/
    Entity sender;
/*@ [SoundStreamFinishedEvent/_] `SoundStreamFinishedEvent.recipient`
    TODO @*/    
    Entity recipient;

    friend std::ostream& operator<<(std::ostream& out, const SoundStreamFinishedEvent & e) {
        out << "SoundStreamFinished["
            << "sender:" << e.sender << ", "
            << "recipient:" << e.recipient
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<SoundStreamFinishedEvent>("SoundStreamFinishedEvent")
                .addData("sender", &SoundStreamFinishedEvent::sender, false)
                .addData("recipient", &SoundStreamFinishedEvent::recipient, false)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
                .addStaticInteger("EventId", Component::EventInfo<SoundStreamFinishedEvent>::GetClassId())
            .endClass();
    }
};

}
