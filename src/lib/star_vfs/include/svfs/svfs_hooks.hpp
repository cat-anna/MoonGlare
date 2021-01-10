#pragma once

#include "svfs/vfs_container.hpp"

namespace MoonGlare::StarVfs {

class iStarVfsHooks {
public:
    virtual ~iStarVfsHooks() = default;

    virtual void OnContainerMounted(iVfsContainer *container) = 0;
};

} // namespace MoonGlare::StarVfs
