#pragma once

namespace Utils {
namespace Strings {

wstring towstring(const string &in);
string tostring(const string &in);

#if 0

template<class T>
inline wstring towstring(T&& in) {
	//std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
	//return convert.from_bytes(std::forward<T>(in));

	return boost::locale::conv::to_utf<wstring::value_type>(in.c_str(), std::locale::classic());
}

template<class T>
inline string tostring(T&& in) {
	//std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
	//return convert.to_bytes(std::forward<T>(in));
	return boost::locale::conv::from_utf<string::value_type>(in.c_str(), std::locale::classic());
}

#endif

}
}