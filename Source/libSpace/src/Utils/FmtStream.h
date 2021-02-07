
#pragma once

#include "../Configuration.h"

namespace Space {

template<typename STREAM>
class BaseFmtStream {
public:
	BaseFmtStream(STREAM & str) : m_stream(str) {}

	template<typename T>
	BaseFmtStream& operator << (T && t) {
		m_stream << t;
		return *this;
	}

	template<typename ... Ts>
	BaseFmtStream& format(const char * fmt, Ts ... ts) {
		char buf[4096];
		sprintf_s(buf, fmt, std::forward<Ts>(ts)...);
		*this << buf;
		return *this;
	}
	template<typename ... Ts>
	BaseFmtStream& line(const char * fmt, Ts ... ts) {
		format(fmt, std::forward<Ts>(ts)...);
		*this << "\n";
		return *this;
	}
private:
	STREAM &m_stream;
};

using OFmtStream = BaseFmtStream<std::ostream>;

}