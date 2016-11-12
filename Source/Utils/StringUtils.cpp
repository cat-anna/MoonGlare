#include <pch.h>
#include <utf8.h>

namespace Utils {
namespace Strings {

wstring towstring(const string &in) {
	wstring r;
	utf8::utf8to32(in.begin(), in.end(), std::back_inserter(r));
	return std::move(r);
}

string tostring(const wstring &in) {
	string r;
	utf8::utf32to8(in.begin(), in.end(), std::back_inserter(r));
	return std::move(r);
}

}
}