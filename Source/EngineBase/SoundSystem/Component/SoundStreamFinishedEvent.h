#pragma once

#include <EngineBase/Scripts/ApiInit.h>

namespace MoonGlare::SoundSystem::Component {

//TODO: StreamStartedEvent 

struct SoundStreamFinishedEvent {
    static constexpr char* EventName = "SoundStreamFinished";
    static constexpr char* HandlerName = "OnSoundStreamFinishedEvent";

    Entity sender;
    Entity recipient;

    friend std::ostream& operator<<(std::ostream& out, const SoundStreamFinishedEvent & e) {
        out << "SoundStreamFinished["
            << "owner:" << e.sender << ", "
            << "recipient:" << e.recipient
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<SoundStreamFinishedEvent>("SoundStreamFinishedEvent")
                .addData("Sender", &SoundStreamFinishedEvent::sender, false)
                .addData("Recipient", &SoundStreamFinishedEvent::recipient, false)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

}
