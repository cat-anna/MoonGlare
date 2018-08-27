#pragma once

#include <Foundation/InterfaceMap.h>
#include <Foundation/Component/EventDispatcher.h>
#include <Renderer/iAsyncLoader.h>
#include <Renderer/Renderer.h>

#include "Renderer.Events.h"

namespace MoonGlare::Renderer {

class RendererEventObserver : public iAsyncLoaderObserver {
public:
    RendererEventObserver(InterfaceMap &ifaceMap) {
        dispatcher = ifaceMap.GetInterface<Component::EventDispatcher>();
        assert(dispatcher);
    }
          
    void OnFinished(iAsyncLoader *loader) override {
        assert(dispatcher);
        dispatcher->Queue(RendererResourceLoaderEvent{false});
    };
    void OnStarted(iAsyncLoader *loader) override {
        assert(dispatcher);
        dispatcher->Queue(RendererResourceLoaderEvent{ true });
    };
private:
    Component::EventDispatcher *dispatcher = nullptr;
};

}
