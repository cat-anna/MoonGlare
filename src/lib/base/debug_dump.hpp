#pragma once

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <string>

namespace MoonGlare {

constexpr auto kDebugDumpPath = "logs/output";

inline void DebugDump(const std::string &name, const std::string &content) {
#ifdef DEBUG_DUMP
    std::filesystem::create_directories(kDebugDumpPath);
    auto fname = fmt::format("{}/{}", kDebugDumpPath, name);
    std::ofstream(fname) << content;
#endif
}

} // namespace MoonGlare
