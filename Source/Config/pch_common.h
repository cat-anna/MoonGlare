#pragma once

#define NOMINMAX

#include <any>
#include <array>
#include <bitset>
#include <deque>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <variant>
#include <vector>

#include <cstring>
#include <regex>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <chrono>
#include <memory>
#include <random>
#include <type_traits>
#include <typeindex>

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

#include <codecvt>
#include <locale>

using StringVector = std::vector<std::string>;
using StringDeque = std::deque<std::string>;
using StringList = std::list<std::string>;
using StringStringMap = std::unordered_map<std::string, std::string>;

#include <boost/align.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

#pragma warning(push, 0)

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#pragma warning(pop)

using LockGuard = std::lock_guard<std::mutex>;
#define MERGE_(a, b) a##b
#define LABEL_(a) MERGE_(unique_name_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

#define LOCK_MUTEX_MERGE(a, b) a##b
#define LOCK_MUTEX_LABEL(name) LOCK_MUTEX_MERGE(__lock_, name)
#define LOCK_MUTEX_LABEL_UNIQUE LOCK_MUTEX_LABEL(__LINE__)

#define LOCK_MUTEX_NAMED(mutex, name) std::lock_guard<decltype(mutex)> name(mutex)
#define LOCK_MUTEX(mutex) LOCK_MUTEX_NAMED(mutex, LOCK_MUTEX_LABEL_UNIQUE)

#define AS_STRING(X) BOOST_PP_STRINGIZE(X)

#ifdef DEBUG
#define ERROR_STR "{badstr in " __FUNCTION__ " at " AS_STRING(__LINE__) "}"
#else
#define ERROR_STR "{?}"
#endif

#include <Memory/AlignedPtr.h>
#include <Memory/StaticAllocationBuffer.h>
#include <Memory/StaticVector.h>

#include <Foundation/OrbitLoggerConf.h>

#include <dynamic_class_register.h>
#include <libSpace/src/Utils/FmtStream.h>
