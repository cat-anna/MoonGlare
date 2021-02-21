#pragma once

namespace MoonGlare {

class iStepableObject {
public:
    virtual void DoStep(double time_delta) = 0;
};

} // namespace MoonGlare