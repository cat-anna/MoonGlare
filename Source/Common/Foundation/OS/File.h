#pragma once

#include <string>

namespace MoonGlare::OS {

void WriteStringToFile(const std::string &FileName, const std::string &data);

bool GetFileContent(const std::string &FileName, std::string &data);

}