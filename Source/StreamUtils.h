#pragma once

template <class T, glm::precision P>
inline std::ostream& operator << (std::ostream &o, const glm::detail::tvec2<T, P> &t) {
	char b[64];
	sprintf(b, "(%.2f; %.2f)", t[0], t[1]);
	return (o << b);
}

template <class T, glm::precision P>
inline std::ostream& operator << (std::ostream &o, const glm::detail::tvec3<T, P> &t) {
	char b[64];
	sprintf(b, "(%.2f; %.2f; %.2f)", t[0], t[1], t[2]);
	return (o << b);
}

template <class T, glm::precision P>
inline std::ostream& operator << (std::ostream &o, const glm::detail::tvec4<T, P> &t) {
	char b[64];
	sprintf(b, "(%.2f; %.2f; %.2f; %.2f)", t[0], t[1], t[2], t[3]);
	return (o << b);
}

inline std::ostream& operator << (std::ostream &o, const Physics::Quaternion &t) {
	auto axis = t.getAxis();
	auto angle = t.getAngle();
	char b[64];
	sprintf(b, "(%.2f; %.2f; %.2f; [%.2f])", axis[0], axis[1], axis[2], angle);
	return (o << b);
}

inline std::ostream& operator << (std::ostream &o, const Physics::vec3 &t) {
	char b[64];
	sprintf(b, "(%.2f; %.2f; %.2f)", t[0], t[1], t[2]);
	return (o << b);
}

