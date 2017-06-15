#pragma once

#define NOMINMAX

#include <unordered_map>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <array>
#include <bitset>

#include <string>
#include <cstring>
#include <regex>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>

#include <memory>
#include <algorithm>
#include <chrono>
#include <random>
#include <type_traits>

#include <mutex>
#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>

#include <locale>
#include <codecvt>

using StringVector = std::vector < std::string >;
using StringDeque = std::deque < std::string >;
using StringList = std::list < std::string >;
using StringStringMap = std::unordered_map < std::string, std::string >;

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/align.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace mem {

template<class T>
using aligned_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

template<class T, class... Args>
inline aligned_ptr<T> make_aligned(Args&&... args) {
    auto p = boost::alignment::aligned_alloc(boost::alignment::alignment_of<T>::value, sizeof(T));
    if (!p) {
        throw std::bad_alloc();
    }
    try {
        auto q = ::new(p) T(std::forward<Args>(args)...);
        return aligned_ptr<T>(q);
    }
    catch (...) {
        boost::alignment::aligned_free(p);
        throw;
    }
}

}

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

#pragma warning ( push, 0 )

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#pragma warning ( pop )

namespace emath {
    using fvec2 = Eigen::Array2f;
    using fvec3 = Eigen::Array3f;
    using fvec4 = Eigen::Array4f;

    using ivec2 = Eigen::Array2i;
    using ivec3 = Eigen::Array3i;
    using ivec4 = Eigen::Array4i;

    using usvec2 = Eigen::Array<uint16_t, 2, 1>;
    using usvec3 = Eigen::Array<uint16_t, 3, 1>;
    using usvec4 = Eigen::Array<uint16_t, 4, 1>;

    using fmat3 = Eigen::Matrix3f;
    using fmat4 = Eigen::Matrix4f;


    template<class T, class S>
    T MathCast(const S& s) {
        static_assert(std::is_same<int, int>::value, "invalid cast!");
        throw false;
    }

    template<>
    inline fmat4 MathCast(const glm::mat4 &s) {
        return fmat4(&s[0][0]);
    }
    template<>
    inline glm::fmat4 MathCast(const fmat4 &s) {
        return *reinterpret_cast<const glm::fmat4*>(s.data());
    }

#define GEN_2(SRC, DST) 										\
    template<>inline DST ## 2 MathCast(const SRC ## 2 &s) {		\
        return DST ## 2(s[0], s[1]);							\
    }
#define GEN_3(SRC, DST) 										\
    template<>inline DST ## 3 MathCast(const SRC ## 3 &s) {		\
        return DST ## 3(s[0], s[1], s[2]);						\
    }
#define GEN_4(SRC, DST) 										\
    template<>inline DST ## 4  MathCast(const SRC ## 4 &s) {	\
        return DST ## 4 (s[0], s[1], s[2], s[3]);				\
    }

#define GEN(SRC, DST) GEN_2(SRC, DST) GEN_3(SRC, DST) GEN_4(SRC, DST) 

#pragma warning ( push, 0 )
    GEN(glm::fvec, fvec);
    GEN(fvec, glm::fvec);
    GEN(glm::ivec, ivec);
    GEN(glm::fvec, ivec);
    GEN(usvec, ivec);
    GEN(usvec, glm::fvec);
#pragma warning ( pop )

#undef GEN
#undef GEN_1
#undef GEN_2
#undef GEN_3
#undef GEN_4
}

using LockGuard = std::lock_guard < std::mutex >;
#define MERGE_(a,b)  a##b
#define LABEL_(a) MERGE_(unique_name_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

#define LOCK_MUTEX_MERGE(a, b) a ## b
#define LOCK_MUTEX_LABEL(name) LOCK_MUTEX_MERGE(__lock_, name)
#define LOCK_MUTEX_LABEL_UNIQUE LOCK_MUTEX_LABEL(__LINE__)

#define LOCK_MUTEX_NAMED(mutex, name) std::lock_guard < decltype(mutex) > name (mutex)
#define LOCK_MUTEX(mutex) LOCK_MUTEX_NAMED(mutex, LOCK_MUTEX_LABEL_UNIQUE)

#ifdef WINDOWS
#include "PlatformWindows.h"
#else
#error unknown platform!
#endif

#define DISABLE_COPY() public: ThisClass(const ThisClass&) = delete; ThisClass& operator=(const ThisClass&) = delete
#define CriticalCheck(COND, MSG)					do { if(!(COND)) { AddLogf(Error, "Critical check failed!!! condition '%s' returned false. Error message: '%s'", #COND, (MSG?MSG:"No error message")); throw MSG; } } while(0)

#define AS_STRING(X) BOOST_PP_STRINGIZE(X)

#ifdef DEBUG
#define ERROR_STR				"{badstr in " __FUNCTION__ " at " AS_STRING(__LINE__) "}"
#else
#define ERROR_STR				"{?}"
#endif

#ifdef GLOBAL_CONFIGURATION_FILE
#include GLOBAL_CONFIGURATION_FILE
#endif

#include <libSpace/src/Memory/BitampAllocator.h>
#include <libSpace/src/Container/StaticVector.h>
#include <libSpace/src/Container/StaticAllocationBuffer.h>
#include <libSpace/src/Utils/FmtStream.h>
