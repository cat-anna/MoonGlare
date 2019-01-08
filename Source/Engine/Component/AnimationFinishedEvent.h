#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/StaticString.h>
#include <Foundation/Component/EventInfo.h>

namespace MoonGlare::Component {

/*@ [ScriptObjectEventHandler/_] ScriptObject::OnAnimationFinished(animationFinishedEventData)
    This method is called when entity receives this event. See definition of this event for details.
@*/

/*@ [EventsReference/AnimationFinishedEvent] AnimationFinishedEvent
    This event is send to entity when BoneAnimatorComponent finishes or loops playback.
@*/
struct AnimationFinishedEvent {
    static constexpr char* EventName = "AnimationFinished";
    static constexpr char* HandlerName = "OnAnimationFinished";
    static constexpr bool Public = false;

    using AnimationName = BasicStaticString<64, char>;

/*@ [AnimationFinishedEvent/_] `AnimationFinishedEvent.sender`
    TODO @*/
    Entity sender;
/*@ [AnimationFinishedEvent/_] `AnimationFinishedEvent.recipient`
    TODO @*/    
    Entity recipient;
/*@ [AnimationFinishedEvent/_] `AnimationFinishedEvent.loop`
    TODO @*/       
    bool loop;

    friend std::ostream& operator<<(std::ostream& out, const AnimationFinishedEvent & e) {
        out << "AnimationFinished["
            << "sender:" << e.sender << ", "
            << "recipient:" << e.recipient << ", "
            << "loop:" << (int)e.loop << ", "
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<AnimationFinishedEvent>("AnimationFinished")
                .addData("sender", &AnimationFinishedEvent::sender, false)
                .addData("recipient", &AnimationFinishedEvent::recipient, false)
                .addData("loop", &AnimationFinishedEvent::loop, false)
            
            //TODO:move event common api part to some define?
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
                .addStaticInteger("EventId", Component::EventInfo<AnimationFinishedEvent>::GetClassId())
            .endClass();
    }
};

}
