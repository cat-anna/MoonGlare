#pragma once

#include <string>
#include <list>
#include <functional>

namespace MoonGlare::OS {
    int WaitForProcess(const std::list<std::string> &arguments, const std::list<std::string> &inputLines, std::function<void(std::string)> onOutput);

    inline int WaitForProcess(const std::list<std::string> &arguments, std::function<void(std::string)> onOutput) {
        std::list<std::string> input;
        return WaitForProcess(arguments, input, onOutput);
    }

    inline int WaitForProcess(const std::list<std::string> &arguments) {
        std::list<std::string> input;
        return WaitForProcess(arguments, input, [](std::string) {});
    }
}