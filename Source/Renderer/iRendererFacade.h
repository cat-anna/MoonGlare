#pragma once

#include "iContext.h"

namespace MoonGlare::Renderer {

class iRendererFacade {
public:

    virtual iContext* GetContext() = 0;
};

}
