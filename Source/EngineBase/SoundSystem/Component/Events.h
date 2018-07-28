#pragma once

#include <EngineBase/Script/ApiInit.h>

namespace MoonGlare::SoundSystem::Component {

struct SoundStreamFinished {
    static constexpr char* EventName = "OnSoundStreamFinished";
    static constexpr char* HandlerName = "OnSoundStreamFinishedEvent";

    bool loop;

    friend std::ostream& operator<<(std::ostream& out, const SoundStreamFinished & e) {
        out << "SoundStreamFinished["
            << "loop:" << (int)e.loop
            << "]";
        return out;
    }

    static Script::ApiInitializer RegisterScriptApi(Script::ApiInitializer api) {
        return api
            .beginClass<SoundStreamFinished>("SoundStreamFinished")
                .addData("loop", &SoundStreamFinished::loop, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

}
