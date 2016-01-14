#pragma once

namespace Utils {
namespace Strings {

template<class T>
inline wstring towstring(T&& in) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
	return convert.from_bytes(std::forward<T>(in));
}

template<class T>
inline string tostring(T&& in) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
	return convert.to_bytes(std::forward<T>(in));
}

}
}