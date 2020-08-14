#pragma once

#include <string_view>
#include <tuple>

namespace MoonGlare::StarVfs {

inline std::tuple<std::string_view, std::string_view> GetParentAndFileName(const std::string_view &path) {
    auto last_slash = path.find_last_of("/");
    if (last_slash == std::string_view::npos) {
        return {{}, path};
    }
    return {path.substr(0, last_slash), path.substr(last_slash + 1)};
}

inline std::string OptimizeMountPointPath(std::string mount_point) {
    while (!mount_point.empty() && mount_point.back() == '/') {
        mount_point.pop_back();
    }
    return mount_point;
}

inline std::string CheckPath(std::string path) {
    if (path == "/") {
        return "";
    }
    return path;
}

template <typename A, typename B> //
inline std::string JoinPath(const A &mount_point, const B &sub_path) {
    if (mount_point.empty())
        return std::string(sub_path);
    return std::string(mount_point) + "/" + std::string(sub_path);
}

} // namespace MoonGlare::StarVfs
