#pragma once

#include <string_view>
#include <tuple>

namespace MoonGlare::StarVfs {

inline std::tuple<std::string_view, std::string_view>
GetParentAndFileName(const std::string_view &path) {
    auto last_slash = path.find_last_of("/");
    if (last_slash == std::string_view::npos) {
        return {{}, path};
    }
    return {path.substr(0, last_slash), path.substr(last_slash + 1)};
}

inline std::string CheckPath(std::string path) {
    if (path.empty()) {
        return "/";
    }
    return path;
}

inline std::string OptimizeMountPointPath(std::string mount_point) {
    while (!mount_point.empty() && mount_point.back() == '/') {
        mount_point.pop_back();
    }
    return CheckPath(mount_point);
}

template <typename A, typename B>
inline std::string JoinPath(const A &mount_point, const B &sub_path) {
    std::string r = mount_point;
    if (r.empty() || r.back() != '/') {
        r += "/";
    }
    if (!sub_path.empty() && sub_path.front() == '/') {
        r.pop_back();
    }
    r += sub_path;
    return r;
}

template <typename T>
T GetExtension(const T &input) {
    auto dot_pos = input.find_last_of(".");
    if (dot_pos == T::npos) {
        return T("");
    }
    return T(input.substr(dot_pos));
}

} // namespace MoonGlare::StarVfs
