#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include "ComponentInfo.h"

namespace MoonGlare::Component {

//TODO: ComponentDestructedEvent

struct ComponentCreatedEvent {
    static constexpr char* EventName = "ComponentCreated";
    static constexpr char* HandlerName = "OnComponentCreatedEvent";

    Entity sender;
    Entity recipient;
    ComponentClassId ccid;

    friend std::ostream& operator<<(std::ostream& out, const ComponentCreatedEvent & e) {
        out << "SoundStreamFinished["
            << "ccid:" << (int)e.ccid << ", "
            << "owner:" << e.sender << ", "
            << "recipient:" << e.recipient
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<ComponentCreatedEvent>("ComponentCreatedEvent")
                .addData("Sender", &ComponentCreatedEvent::sender, false)
                .addData("Recipient", &ComponentCreatedEvent::recipient, false)

                .addProperty("ClassId", &ComponentCreatedEvent::getClassId)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }

private:
    int getClassId() const { return static_cast<int>(ccid); }
};

}
