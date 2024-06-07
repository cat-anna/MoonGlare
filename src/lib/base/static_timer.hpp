#pragma once

#include <chrono>

namespace MoonGlare {

class StaticTimer {
public:
    using clock_t = std::chrono::steady_clock;

    StaticTimer(std::chrono::milliseconds _interval) : interval(_interval), deadline(clock_t::now() + interval) {}

    template <typename F>
    void Check(F f, clock_t::time_point time = clock_t::now()) {
        if (time > deadline) {
            deadline = time + interval;
            f();
        }
    }

private:
    const std::chrono::milliseconds interval;
    clock_t::time_point deadline;
};

} // namespace MoonGlare
