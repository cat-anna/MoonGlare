#pragma once

#include <stdexcept>
#include <string>

namespace MoonGlare::Scripts {

class LuaPanic : public std::runtime_error {
public:
    LuaPanic(const std::string &msg) : std::runtime_error(msg) {
#ifdef DEBUG
        __debugbreak();
#endif
    }
};

}