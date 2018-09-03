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
#include <variant>      
#include <optional>
#include <any>

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
#include <cassert>

#include <memory>
#include <algorithm>
#include <chrono>
#include <random>
#include <type_traits>
#include <typeindex>

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

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

#pragma warning ( push, 0 )

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#pragma warning ( pop )
                                 
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

#define CriticalCheck(COND, MSG)					do { if(!(COND)) { AddLogf(Error, "Critical check failed!!! condition '%s' returned false. Error message: '%s'", #COND, (MSG?MSG:"No error message")); throw MSG; } } while(0)

#define AS_STRING(X) BOOST_PP_STRINGIZE(X)

#ifdef DEBUG
#define ERROR_STR				"{badstr in " __FUNCTION__ " at " AS_STRING(__LINE__) "}"
#else
#define ERROR_STR				"{?}"
#endif

#include <Foundation/Memory/AlignedPtr.h>
#include <Foundation/OrbitLoggerConf.h>

#include <libSpace/src/Container/StaticVector.h>
#include <libSpace/src/Container/StaticAllocationBuffer.h>
#include <libSpace/src/Utils/FmtStream.h>
#include <libSpace/src/Utils/DynamicClassRegister.h>
