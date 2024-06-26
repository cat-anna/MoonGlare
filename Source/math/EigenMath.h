#pragma once

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace emath {

template <class T, class S>
T MathCast(const S &s) {
    static_assert(std::is_same<int, float>::value, "invalid cast!");
    throw false;
}

template <>
inline fmat4 MathCast(const glm::mat4 &s) {
    return fmat4(&s[0][0]);
}
template <>
inline glm::fmat4 MathCast(const fmat4 &s) {
    return *reinterpret_cast<const glm::fmat4 *>(s.data());
}

template <>
inline glm::fvec4 MathCast(const Quaternion &s) {
    return {
        s.x(),
        s.y(),
        s.z(),
        s.w(),
    };
}

template <>
inline Quaternion MathCast(const glm::fvec4 &s) {
    return {
        s.w,
        s.x,
        s.y,
        s.z,
    };
}

#ifdef BT_VECTOR3_H
template <>
inline fvec3 MathCast(const btVector3 &s) {
    return {
        s.x(),
        s.y(),
        s.z(),
    };
}
template <>
inline Quaternion MathCast(const btQuaternion &s) {
    return {
        s.w(),
        s.x(),
        s.y(),
        s.z(),
    };
}

template <>
inline btVector3 MathCast(const fvec3 &s) {
    return {
        s.x(),
        s.y(),
        s.z(),
    };
}
template <>
inline btQuaternion MathCast(const Quaternion &s) {
    return {
        s.w(),
        s.x(),
        s.y(),
        s.z(),
    };
}
#endif

#define GEN_2(SRC, DST)                                                                            \
    template <>                                                                                    \
    inline DST##2 MathCast(const SRC##2 & s) {                                                     \
        return DST##2(s[0], s[1]);                                                                 \
    }
#define GEN_3(SRC, DST)                                                                            \
    template <>                                                                                    \
    inline DST##3 MathCast(const SRC##3 & s) {                                                     \
        return DST##3(s[0], s[1], s[2]);                                                           \
    }
#define GEN_4(SRC, DST)                                                                            \
    template <>                                                                                    \
    inline DST##4 MathCast(const SRC##4 & s) {                                                     \
        return DST##4(s[0], s[1], s[2], s[3]);                                                     \
    }

#define GEN(SRC, DST) GEN_2(SRC, DST) GEN_3(SRC, DST) GEN_4(SRC, DST)

#pragma warning(push, 0)
GEN(glm::fvec, fvec);
GEN(fvec, glm::fvec);
GEN(glm::ivec, ivec);
GEN(glm::fvec, ivec);
GEN(usvec, ivec);
GEN(usvec, glm::fvec);
#pragma warning(pop)

#undef GEN
#undef GEN_1
#undef GEN_2
#undef GEN_3
#undef GEN_4
} // namespace emath

namespace math {
typedef glm::fvec2 vec2;
typedef glm::fvec3 vec3;
typedef glm::fvec4 vec4;

typedef glm::fvec2 fvec2;
typedef glm::fvec3 fvec3;
typedef glm::fvec4 fvec4;

typedef glm::fmat4 mat4;

typedef glm::uvec2 uvec2;
typedef glm::uvec3 uvec3;
typedef glm::uvec4 uvec4;

#ifdef BT_VECTOR3_H
using Transform = btTransform;
using Quaternion = btQuaternion;
#endif

template <class T>
T min(T a, T b) {
    return a < b ? a : b;
}

template <class T>
T max(T a, T b) {
    return a < b ? b : a;
}

struct RawVec3 {
    union {
        struct {
            float r, g, b;
        };
        struct {
            float x, y, z;
        };
        float m_Raw[3];
    };

    operator const math::fvec3 &() const { return reinterpret_cast<const math::fvec3 &>(*this); }
    void operator=(const math::fvec3 &other) { reinterpret_cast<math::fvec3 &>(*this) = other; }
    void operator=(const emath::fvec3 &other) {
        reinterpret_cast<math::fvec3 &>(*this) = emath::MathCast<math::fvec3>(other);
    }
    void operator=(const float *mat3) { memcpy(m_Raw, mat3, sizeof(m_Raw)); }

    float &operator[](size_t idx) { return m_Raw[idx]; };
    const float &operator[](size_t idx) const { return m_Raw[idx]; };

    float Max() const { return fmax(fmax(r, g), b); }
};
static_assert(std::is_pod<RawVec3>::value, "RawVec3 shall be POD!");
static_assert(sizeof(RawVec3) == sizeof(math::fvec3), "RawVec3 and fvec3 size mismatch!");

struct RawVec4 {
    union {
        struct {
            float r, g, b, a;
        };
        struct {
            float x, y, z, w;
        };
        float m_Raw[4];
    };

    operator const math::fvec4 &() const { return reinterpret_cast<const math::fvec4 &>(*this); }
    void operator=(const math::fvec4 &other) { reinterpret_cast<math::fvec4 &>(*this) = other; }
    void operator=(const emath::fvec4 &other) {
        reinterpret_cast<math::fvec4 &>(*this) = emath::MathCast<math::fvec4>(other);
    }
    void operator=(const float *mat4) { memcpy(m_Raw, mat4, sizeof(m_Raw)); }

    float &operator[](size_t idx) { return m_Raw[idx]; };
    const float &operator[](size_t idx) const { return m_Raw[idx]; };

    float Max() const { return fmax(fmax(r, g), fmax(b, a)); }
};
static_assert(std::is_pod<RawVec4>::value, "RawVec4 shall be POD!");
static_assert(sizeof(RawVec4) == sizeof(math::fvec4), "RawVec4 and fvec3 size mismatch!");

struct RawMat4 {
    RawVec4 m_Mat[4];

    operator const math::mat4 &() const { return reinterpret_cast<const math::mat4 &>(*this); }
    void operator=(const float *mat16) { memcpy(m_Mat, mat16, sizeof(m_Mat)); }
    void operator=(const math::mat4 &other) { reinterpret_cast<math::mat4 &>(*this) = other; }
    void operator=(const emath::fmat4 &other) {
        reinterpret_cast<math::mat4 &>(*this) = emath::MathCast<math::mat4>(other);
    }

    RawVec4 &operator[](size_t idx) { return m_Mat[idx]; };
    const RawVec4 &operator[](size_t idx) const { return m_Mat[idx]; };
};
static_assert(std::is_pod<RawMat4>::value, "RawMat4 shall be POD!");
static_assert(sizeof(RawMat4) == sizeof(math::mat4), "RawMat4 and mat4 size mismatch!");

using RGB = RawVec3;
using RGBS = RawVec4;
} // namespace math

#ifdef BT_VECTOR3_H

inline btVector3 convert(const math::vec3 &src) {
    return btVector3(src[0], src[1], src[2]);
}
inline math::vec3 convert(const btVector3 &src) {
    return math::vec3(src[0], src[1], src[2]);
}

inline math::Quaternion convert(const math::vec4 &src) {
    return math::Quaternion(src[0], src[1], src[2], src[3]);
}
inline math::vec4 convert(const math::Quaternion &src) {
    return math::vec4(src[0], src[1], src[2], src[3]);
}

#endif

using math::uvec2;
using math::uvec3;
using math::vec2;
using math::vec4;

using math::max;
using math::min;
