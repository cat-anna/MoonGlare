#pragma once

#include <Foundation/InterfaceMap.h>
#include <Foundation/Component/EventDispatcher.h>

#include "iAsyncLoader.h"
#include "ResourceLoaderEvent.h"

namespace MoonGlare::Resources {

class AsyncLoaderEventObserver : public Resources::iAsyncLoaderObserver {
public:
    AsyncLoaderEventObserver(InterfaceMap &ifaceMap) {
        dispatcher = ifaceMap.GetInterface<Component::EventDispatcher>();
        assert(dispatcher);
    }
          
    void OnFinished(Resources::iAsyncLoader *loader) override {
        assert(dispatcher);
        dispatcher->Queue(ResourceLoaderEvent{false, revision.fetch_add(1) });
    };
    void OnStarted(Resources::iAsyncLoader *loader) override {
        assert(dispatcher);
        dispatcher->Queue(ResourceLoaderEvent{ true, revision.fetch_add(1) });
    };
private:
    Component::EventDispatcher *dispatcher = nullptr;
    std::atomic<int> revision = 0;
};

}
