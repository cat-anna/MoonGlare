#pragma once

#include <Foundation/Scripts/ApiInit.h>

namespace MoonGlare::Component {

struct EntityCreatedEvent {
    static constexpr char* EventName = "EntityCreated";
    static constexpr char* HandlerName = "OnEntityCreatedEvent";

    Entity parent;
    Entity entity;

    friend std::ostream& operator<<(std::ostream& out, const EntityCreatedEvent & e) {
        out << "EntityCreatedEvent["
            << "Parent:" << e.parent << ", "
            << "Entity:" << e.entity << ", "
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
        .beginClass<EntityCreatedEvent>("EntityCreatedEvent")
            .addData("Parent", &EntityCreatedEvent::parent, false)
            .addData("Entity", &EntityCreatedEvent::entity, false)

            .addStaticString("EventName", EventName)
            .addStaticString("HandlerName", HandlerName)
        .endClass();
    }
};

struct EntityDestructedEvent {
    static constexpr char* EventName = "EntityDestructed";
    static constexpr char* HandlerName = "OnEntityDestructedEvent";

    Entity parent; 
    Entity entity; 

    friend std::ostream& operator<<(std::ostream& out, const EntityDestructedEvent & e) {
        out << "EntityDestructedEvent["
            << "Parent:" << e.parent << ", "
            << "Entity:" << e.entity << ", "
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
        .beginClass<EntityDestructedEvent>("EntityDestructedEvent")
            .addData("Parent", &EntityDestructedEvent::parent, false)
            .addData("Entity", &EntityDestructedEvent::entity, false)

            .addStaticString("EventName", EventName)
            .addStaticString("HandlerName", HandlerName)
        .endClass();
    }
};

}
