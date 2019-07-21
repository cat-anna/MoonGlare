#pragma once

#include <ostream>
#include <fmt/format.h>

template <class T, glm::precision P>
inline std::ostream& operator << (std::ostream &o, const glm::tvec2<T, P> &t) {
    char b[64];
    sprintf(b, "(%.2f; %.2f)", t[0], t[1]);
    return (o << b);
}

template <class T, glm::precision P>
inline std::ostream& operator << (std::ostream &o, const glm::tvec3<T, P> &t) {
    char b[64];
    sprintf(b, "(%.2f; %.2f; %.2f)", t[0], t[1], t[2]);
    return (o << b);
}

template <class T, glm::precision P>
inline std::ostream& operator << (std::ostream &o, const glm::tvec4<T, P> &t) {
    char b[64];
    sprintf(b, "(%.2f; %.2f; %.2f; %.2f)", t[0], t[1], t[2], t[3]);
    return (o << b);
}
