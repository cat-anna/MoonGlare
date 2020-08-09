#pragma once

namespace MoonGlare {

template<typename TIMEPOINT>
double TimeDiff(const TIMEPOINT& start, const TIMEPOINT& end) {
    return std::chrono::duration<double>(end - start).count();
}

}
