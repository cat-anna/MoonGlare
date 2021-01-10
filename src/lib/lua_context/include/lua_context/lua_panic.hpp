#pragma once

#include <debugger_support.hpp>
#include <stdexcept>
#include <string>

namespace MoonGlare::Lua {

class LuaPanic : public std::runtime_error {
public:
    LuaPanic(const std::string &msg) : std::runtime_error(msg) { TriggerBreakPoint(); }
};

} // namespace MoonGlare::Lua