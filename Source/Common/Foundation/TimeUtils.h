#pragma once

namespace MoonGlare {

template<typename TIMEPOINT>
double TimeDiff(TIMEPOINT&& start, TIMEPOINT&& end) {
    return std::chrono::duration<double>(end - start).count();
}

}
