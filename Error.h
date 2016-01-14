#pragma once

namespace MoonGlare {

class CriticalException : public std::exception {
public:
	template <class T>
	CriticalException(T&& msg) : m_msg(msg) {}

	CriticalException(const char* fmt, ...) : m_msg() {
		char buffer[4096];
		va_list args;
		va_start (args, fmt);
		vsnprintf (buffer, sizeof(buffer), fmt, args);
		AddLog(Error, buffer);
		m_msg = buffer;
		va_end (args);
	}

	const char* what() const override {
		return m_msg.c_str();
	}
private:
	string m_msg;
};

#define CriticalError(fmt, ...) do { throw ::MoonGlare::CriticalException(fmt, __VA_ARGS__); } while(false)

} //namespace MoonGlare
