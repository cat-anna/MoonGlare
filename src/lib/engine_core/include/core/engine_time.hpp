#pragma once

#include <chrono>

namespace MoonGlare {

class iEngineTime {
public:
    virtual ~iEngineTime() = default;

    using clock_t = std::chrono::steady_clock;

    virtual double GetGlobalTime() const = 0;
    virtual double GetCurrentTimeDelta() const = 0;
    virtual size_t GetFrameCounter() const = 0;
    virtual clock_t::time_point GetStartTime() const = 0;
};

} // namespace MoonGlare