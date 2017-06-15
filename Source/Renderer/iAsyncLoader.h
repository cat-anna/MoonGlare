#pragma once

namespace MoonGlare::Renderer {

class iAsyncLoader {
public:
    virtual ~iAsyncLoader() {};

    virtual bool AnyJobPending() = 0;
    virtual bool AllResoucecsLoaded() = 0;

};

} //namespace MoonGlare::Renderer
