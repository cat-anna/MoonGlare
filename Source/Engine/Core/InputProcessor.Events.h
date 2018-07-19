#pragma once

namespace MoonGlare::Core {

struct InputProcessorOnCharEvent {
    static constexpr char* EventName = "OnChar";
    static constexpr char* HandlerName = "OnCharEvent";
   
    int m_Char;

    friend std::ostream& operator<<(std::ostream& out, const InputProcessorOnCharEvent & e) {
        out << "InputProcessorOnCharEvent"
            << "[char:" << (int)e.m_Char
            << "]";
        return out;
    }

    static ApiInitializer RegisterLuaApi(ApiInitializer api) {
        return api
            .beginClass<InputProcessorOnCharEvent>("InputProcessorOnCharEvent")
                .addData("char", &InputProcessorOnCharEvent::m_Char, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

struct InputProcessorOnKeyEvent {
    static constexpr char* EventName = "OnKey";
    static constexpr char* HandlerName = "OnKeyEvent";

    int m_Key;

    friend std::ostream& operator<<(std::ostream& out, const InputProcessorOnKeyEvent & e) {
        out << "InputProcessorOnKeyEvent"
            << "[key:" << (int)e.m_Key
            << "]";
        return out;
    }

    static ApiInitializer RegisterLuaApi(ApiInitializer api) {
        return api
            .beginClass<InputProcessorOnKeyEvent>("InputProcessorOnKeyEvent")
                .addData("key", &InputProcessorOnKeyEvent::m_Key, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

} //namespace MoonGlare::Core

