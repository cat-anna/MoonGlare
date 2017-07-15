#pragma once

#include "CommandQueueBase.h"
#include "../Resources/MeshResource.h"

namespace MoonGlare::Renderer::Commands {

namespace detail {

struct CommitMesh {
    Resources::MeshManager::HandleType handle;
    Resources::MeshManager &meshManager;
 
    void Run() {
        meshManager.CommitMesh(handle);
    }
};

}

using CommitMesh = RunnableCommandTemplate<detail::CommitMesh>;

} //namespace MoonGlare::Renderer::Commands
