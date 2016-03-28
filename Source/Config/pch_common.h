#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include <cstring>

#include <memory>

#include <mutex>
#include <thread>

#ifdef WINDOWS
#include "PlatformWindows.h"
#else
#error unknown platform!
#endif
