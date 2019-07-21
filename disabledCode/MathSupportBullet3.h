#pragma once

#include <ostream>
#include <fmt/format.h>

inline std::ostream& operator << (std::ostream &o, const btVector3& v) {
    o << fmt::format("({};{};{})", v.x(), v.y(), v.z());
    return o;
}

inline std::ostream& operator << (std::ostream &o, const btQuaternion &t) {
    auto axis = t.getAxis();
    auto angle = t.getAngle();
    char b[64];
    sprintf(b, "(%.2f; %.2f; %.2f; [%.2f])", axis[0], axis[1], axis[2], angle);
    return (o << b);
}
