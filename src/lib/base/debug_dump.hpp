#pragma once

#include "build_configuration.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <string>

namespace MoonGlare {

constexpr auto kDebugDumpPath = "logs/output";

inline void DebugDump(const std::string &name, const std::string &content) {
    if constexpr (kDebugDumpEnabled) {
        std::filesystem::create_directories(kDebugDumpPath);
        auto fname = fmt::format("{}/{}", kDebugDumpPath, name);
        std::ofstream(fname) << content;
    }
}

} // namespace MoonGlare
