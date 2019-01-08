#pragma once

#include <Foundation/InterfaceMap.h>

namespace MoonGlare::Renderer {

class iRenderDevice {
public:
    virtual ~iRenderDevice() {}

    virtual void SetCaptureScreenShoot() = 0;
};

}
