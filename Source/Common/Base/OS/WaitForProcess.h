#pragma once

#include <string>
#include <list>
#include <functional>

namespace MoonGlare::Base::OS {
    int WaitForProcess(const std::list<std::string> &arguments, const std::list<std::string> &inputLines, std::function<void(std::string)> onOutput);
}