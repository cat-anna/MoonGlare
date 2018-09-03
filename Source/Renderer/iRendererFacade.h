#pragma once

#include <Foundation/InterfaceMap.h>

#include "iContext.h"

namespace MoonGlare::Renderer {

class iRendererFacade {
public:
    virtual ~iRendererFacade() { }

    virtual void Initialize(const ContextCreationInfo& ctxifo, iFileSystem *fileSystem) = 0;
    virtual void Finalize() = 0;

    virtual iContext* GetContext() = 0;

    static std::unique_ptr<iRendererFacade> CreateInstance(InterfaceMap &ifaceMap);
};

using UniqueRenderer = std::unique_ptr<iRendererFacade>;

}
