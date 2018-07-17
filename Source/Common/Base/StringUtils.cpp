#include "StringUtils.h"
#include <utf8.h>

namespace Utils {
namespace Strings {

std::wstring towstring(const std::string &in) {
    std::wstring r;
    utf8::utf8to16(in.begin(), in.end(), std::back_inserter(r));
    return std::move(r);
}

std::string tostring(const std::wstring &in) {
    std::string r;
    utf8::utf16to8(in.begin(), in.end(), std::back_inserter(r));
    return std::move(r);
}

}
}