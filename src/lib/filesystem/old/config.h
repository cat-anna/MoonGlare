#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace StarVFS {

struct Settings {
    struct Initial {
        enum {
            FileTableSize = 1 * 1024,
            StringTableSize = 4 * 1024,
            HandleTableSize = 1 * 1024,
        };
    };
};

using FilePathHash = uint32_t;
using FileTypeHash = uint32_t;
using FileSize = uint32_t;
using FileID = uint16_t; // value 0 means invalid id
using ContainerID = uint8_t;
using StringID = uint32_t;

using Char = char;
using String = std::string; // std::basic_string<Char>;
using CString = Char *;
using ConstCString = const Char *;

using DynamicFIDTable = std::vector<FileID>;

extern void (*StarVFSLogSink)(const char *file, const char *function, unsigned line, const char *log, const char *type);

} // namespace StarVFS

#if defined(GCC) && !defined(sprintf_s)
#define sprintf_s snprintf
#endif

#ifdef STARVFS_LOG_TO_SINK

#define STARVFS_DEFINE_DEFAULT_LOG_API

#define STARVFSLOG(What, fmt, ...)                                                                                     \
    do {                                                                                                               \
        char __logbuf[4096];                                                                                           \
        sprintf(__logbuf, fmt, __VA_ARGS__);                                                                           \
        ::StarVFS::StarVFSLogSink(__FILE__, __FUNCTION__, __LINE__, __logbuf, #What);                                  \
    } while (0)

#elif defined(STARVFS_USE_ORBITLOGGER)

#include <orbit_logger.h>

#define STARVFSLOG(What, fmt, ...) AddLogf(StarVFS, fmt, __VA_ARGS__)

#if defined(DEBUG) || defined(STARVFS_ENABLE_DEBUG_LOG)
#define STARVFSDebugLog(fmt, ...) AddLogf(Debug, "[StarVFS] " fmt, __VA_ARGS__)
#else
#define STARVFSDebugLog(fmt, ...)                                                                                      \
    do {                                                                                                               \
    } while (0)
#endif

#define STARVFSErrorLog(fmt, ...) AddLogf(Debug, "[StarVFS] " fmt, __VA_ARGS__)

#else

#define STARVFS_DEFINE_DEFAULT_LOG_API

#include <iostream>

#define STARVFSLOG(What, args...)                                                                                      \
    do {                                                                                                               \
        char __logbuf[1024 * 16];                                                                                      \
        int c = sprintf(__logbuf, "[%s][%s:%s:%d] StarVFS: ", #What, __FILE__, __FUNCTION__, __LINE__);                \
        sprintf(__logbuf + c, args);                                                                                   \
        std::cout << __logbuf << std::flush;                                                                           \
    } while (0)

#endif

#ifndef STARVFSErrorLog
#define STARVFSErrorLog(...) STARVFSLOG(Error, __VA_ARGS__)
#endif

#ifndef STARVFSDebugLog
#if defined(DEBUG) || defined(STARVFS_ENABLE_DEBUG_LOG)
#define STARVFSDebugLog(...) STARVFSLOG(debug, __VA_ARGS__)
#else
#define STARVFSDebugLog(fmt, ...)                                                                                      \
    do {                                                                                                               \
    } while (0)
#endif
#endif

#define STARVFSDebugInfoLog STARVFSDebugLog

#define StarVFSAssert assert
