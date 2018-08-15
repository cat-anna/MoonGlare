#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include "ComponentInfo.h"

namespace MoonGlare::Component {

//TODO: ComponentDestructedEvent

struct ComponentCreatedEvent {
    static constexpr char* EventName = "ComponentCreated";
    static constexpr char* HandlerName = "OnComponentCreatedEvent";

    Entity sender;
    ComponentClassId ccid;

    friend std::ostream& operator<<(std::ostream& out, const ComponentCreatedEvent & e) {
        out << "ComponentCreatedEvent["
            << "ccid:" << (int)e.ccid << ", "
            << "entity:" << e.sender << ", "
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<ComponentCreatedEvent>("ComponentCreatedEvent")
                .addData("Sender", &ComponentCreatedEvent::sender, false)

                .addProperty("ClassId", &ComponentCreatedEvent::getClassId)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }

private:
    int getClassId() const { return static_cast<int>(ccid); }
};

struct ComponentDestructedEvent {
    static constexpr char* EventName = "ComponentDestructed";
    static constexpr char* HandlerName = "OnComponentDestructedEvent";

    Entity sender;
    ComponentClassId ccid;

    friend std::ostream& operator<<(std::ostream& out, const ComponentDestructedEvent & e) {
        out << "ComponentCreatedEvent["
            << "ccid:" << (int)e.ccid << ", "
            << "entity:" << e.sender << ", "
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<ComponentDestructedEvent>("ComponentDestroyedEvent")
            .addData("Sender", &ComponentDestructedEvent::sender, false)

            .addProperty("ClassId", &ComponentDestructedEvent::getClassId)

            .addStaticString("EventName", EventName)
            .addStaticString("HandlerName", HandlerName)
            .endClass();
    }

private:
    int getClassId() const { return static_cast<int>(ccid); }
};

}
