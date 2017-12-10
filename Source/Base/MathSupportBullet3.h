#pragma once

#include <ostream>
#include <fmt/format.h>

std::ostream& operator << (std::ostream &o, const btVector3& v) {
    o << fmt::format("({};{};{})", v.x(), v.y(), v.z());
    return o;
}
