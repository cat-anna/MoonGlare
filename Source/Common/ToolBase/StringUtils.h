
#include <algorithm>
#include <string>

inline std::string ToLower(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return std::move(str);
}