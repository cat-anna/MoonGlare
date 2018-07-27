#pragma once

namespace MoonGlare::SoundSystem::Component {

struct SoundStreamFinished {
    static constexpr char* EventName = "OnSoundStreamFinished";
    static constexpr char* HandlerName = "OnSoundStreamFinishedEvent";

    friend std::ostream& operator<<(std::ostream& out, const SoundStreamFinished & e) {
        out << "SoundStreamFinished"
            //<< "[char:" << (int)e.m_Char
            << "]";
        return out;
    }

    static ApiInitializer RegisterLuaApi(ApiInitializer api) {
        return api
            .beginClass<SoundStreamFinished>("SoundStreamFinished")
                //.addData("char", &SoundStreamFinished::m_Char, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};


}
