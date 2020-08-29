#pragma once

#if 0
#include <Foundation/Scripts/ApiInit.h>

namespace MoonGlare::Resources {

struct ResourceLoaderEvent {
    static constexpr char* EventName = "ResourceLoaderEvent";
    static constexpr char* HandlerName = "OnResourceLoaderEvent";
    static constexpr bool Public = false;

    bool busy;
    //char __padding[4 - sizeof(busy)];
    int revision;

    friend std::ostream& operator<<(std::ostream& out, const ResourceLoaderEvent & dt) {
        out << "RendererResourceLoaderEvent"
            << "[busy:" << (int)dt.busy
            << ",revision:" << dt.revision
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<ResourceLoaderEvent>("ResourceLoaderEvent")
                .addData("Busy", &ResourceLoaderEvent::busy, false)
                .addData("Revision", &ResourceLoaderEvent::revision, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};


}

#endif