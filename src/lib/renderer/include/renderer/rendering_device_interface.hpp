#pragma once

#include "device_context.hpp"
#include "facade.hpp"

namespace MoonGlare::Renderer {

class iRenderingDevice : public iRenderingDeviceFacade {
public:
    virtual ~iRenderingDevice() = default;

    virtual void EnterLoop() = 0;

    virtual void NextFrame() = 0;
    virtual void SubmitFrame() = 0;

    // Frame *NextFrame();
    // void Submit(Frame *frame);
    // void ReleaseFrame(Frame *frame);
    // Frame *PendingFrame();
};

} // namespace MoonGlare::Renderer
