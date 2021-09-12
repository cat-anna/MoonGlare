#pragma once

#if 0

#include "../Resources/Mesh/MeshResource.h"
#include "CommandQueueBase.h"

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

#endif