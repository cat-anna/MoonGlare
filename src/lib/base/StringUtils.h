#pragma once

#include <string>

namespace Utils {
namespace Strings {

std::wstring towstring(const std::string &in);
std::string tostring(const std::wstring &in);

}
}