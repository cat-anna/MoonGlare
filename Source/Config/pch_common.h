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

using StringVector = std::vector < std::string >;
using StringDeque = std::deque < std::string >;
using StringList = std::list < std::string >;
using StringStringMap = std::unordered_map < std::string, std::string >;

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>


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

	using fmat3 = Eigen::Matrix3f;
	using fmat4 = Eigen::Matrix4f;

	template<class T, class S>
	T MathCast(const S& s);

	template<>
	inline fmat4 MathCast(const glm::mat4 &s) {
		return fmat4(&s[0][0]);
	}
	template<>
	inline glm::fmat4 MathCast(const fmat4 &s) {
		return *reinterpret_cast<const glm::fmat4*>(s.data());
	}

	template<>
	inline fvec3 MathCast(const glm::fvec3 &s) {
		return fvec3(s[0], s[1], s[2]);
	}
	template<>
	inline fvec4 MathCast(const glm::fvec4 &s) {
		return fvec4(s[0], s[1], s[2], s[3]);
	}
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
