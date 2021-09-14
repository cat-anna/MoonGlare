#pragma once

namespace MoonGlare::Renderer {

class iFrameSink;
class iRenderingDevice;
class iRenderTarget;
class iResourceManager;

class iRenderingDeviceFacade {
public:
    virtual ~iRenderingDeviceFacade() = default;
    virtual iFrameSink *GetFrameSink() = 0;
    virtual iResourceManager *GetResourceManager() = 0;
};

#ifdef WANTS_GTEST_MOCKS
struct RenderingDeviceFacadeMock : public iRenderingDeviceFacade {
    MOCK_METHOD0(GetFrameSink, iFrameSink *());
    MOCK_METHOD0(GetResourceManager, iResourceManager *());
};
#endif

} // namespace MoonGlare::Renderer
