#pragma once

#include <type_traits>

namespace math {

    template<class T>
    inline T clamp(T t, T tmin, T tmax) {
        if (t > tmax) return tmax;
        if (t < tmin) return tmin;
        return t;
    }

    template<class VEC, class POS>
    inline VEC LinearVectorMix(const VEC &v1, const VEC &v2, POS t) {
        return v1 * t + v2 * (static_cast<POS>(1.0) - t);
    }

    /** Calculate plane normal from 3 points */
    template<class VEC> inline VEC PlaneNormal(const VEC &p1, const VEC &p2, const VEC &p3) {
        return PlaneNormalVectors(p2 - p1, p3 - p1);
    }

    /** Calculate plane normal from 2 points and vector */
    template<class VEC> inline VEC PlaneNormalPointPointVector(const VEC &p1, const VEC &p2, const VEC &v1) {
        return PlaneNormalVectors(p2 - p1, v1);
    }

    /** Calculate plane normal from 2 vectors */
    template<class VEC> inline VEC PlaneNormalVectors(const VEC &v1, const VEC &v2) {
        return v1.cross(v2);
    }

    template<class T>
    inline T next_power2(T v) {
        static_assert(std::is_integral_v<T>);
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }
}
