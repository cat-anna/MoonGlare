#pragma once

#include "iContext.h"

namespace MoonGlare::Renderer {

class iRendererFacade {
public:
    virtual ~iRendererFacade() { }

    virtual void Initialize(const ContextCreationInfo& ctxifo, iFileSystem *fileSystem) = 0;
    virtual void Finalize() = 0;

    virtual iContext* GetContext() = 0;

    virtual void SetConfiguration(Configuration::RuntimeConfiguration Configuration) = 0;
};

}
