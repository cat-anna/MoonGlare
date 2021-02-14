#pragma once

#include <interface_map.h>

namespace MoonGlare::Renderer {

class iRenderDevice {
  public:
    virtual ~iRenderDevice() {}

    virtual void SetCaptureScreenShoot() = 0;
};

} // namespace MoonGlare::Renderer
