#pragma once

namespace MoonGlare::Renderer {

class iRenderingDevice;
class iRenderTarget;
class iResourceManager;

struct CommandQueueRef;

class iRenderingDeviceFacade {
public:
    virtual ~iRenderingDeviceFacade() = default;
    virtual iRenderTarget *GetDisplayRenderTarget() = 0;
    virtual iResourceManager *GetResourceManager() = 0;
};

#ifdef WANTS_GTEST_MOCKS
struct RenderingDeviceFacadeMock : public iRenderingDeviceFacade {
    MOCK_METHOD0(GetDisplayRenderTarget, iRenderTarget *());
    MOCK_METHOD0(GetResourceManager, iResourceManager *());
};
#endif

} // namespace MoonGlare::Renderer
