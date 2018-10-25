#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/StaticString.h>

namespace MoonGlare::Component {

struct AnimationFinishedEvent {
    static constexpr char* EventName = "AnimationFinished";
    static constexpr char* HandlerName = "OnAnimationFinished";
    static constexpr bool Public = false;

    using AnimationName = BasicStaticString<64, char>;

    Entity sender;
    Entity recipient;
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
                .addData("Sender", &AnimationFinishedEvent::sender, false)
                .addData("Recipient", &AnimationFinishedEvent::recipient, false)
                .addData("Loop", &AnimationFinishedEvent::loop, false)
            
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

}
