#pragma once

#include <string>
#include <string_view>

namespace MoonGlare::Lua {

class iCodeChunkRunner {
public:
    virtual ~iCodeChunkRunner() = default;

    virtual bool ExecuteCodeChunk(const char *code, unsigned len, const char *chunk_name = nullptr) = 0;

    bool ExecuteCodeChunk(const std::string &code, const char *chunk_name = nullptr) {
        return ExecuteCodeChunk(code.c_str(), static_cast<unsigned>(code.size()), chunk_name);
    }
    bool ExecuteCodeChunk(const std::string_view &code, const char *chunk_name = nullptr) {
        return ExecuteCodeChunk(code.data(), static_cast<unsigned>(code.size()), chunk_name);
    }
};

constexpr auto kLuaInitScript = "init.lua";

} // namespace MoonGlare::Lua
