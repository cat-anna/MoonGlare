#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include <cstring>

#include <memory>

#include <mutex>
#include <thread>
#include <atomic>

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
