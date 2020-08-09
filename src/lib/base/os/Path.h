#pragma once

#include <string>
#include <list>
#include <functional>

namespace MoonGlare::OS {

std::string GetSettingsDirectory(const std::string &Product = "MoonGlare");
std::string GetSettingsFilePath(const std::string &Name, const std::string &Product = "MoonGlare");

}