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
