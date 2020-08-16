#pragma once

#include <map>
#include <string>

namespace MoonGlare::Tools {

struct CompilationResult {
    enum class Status {
        Success,
        Failed,
        Error,
    } status;
    std::map<unsigned, std::string> lineInfo;
};

class LuaLinter {
public:
    LuaLinter();
    ~LuaLinter();
    CompilationResult Compile(const std::string &code);
};

} // namespace MoonGlare::Tools
