#pragma once

namespace Graphic {
namespace Shaders {

//struct ShaderCode {
//	char *code = 0;
//	ShaderCode(): code (0) { }
//	ShaderCode(ShaderCode&& c) : code(0) { std::swap(c.code, code); }
//	ShaderCode(const ShaderCode& c) = delete;
//	~ShaderCode() {	delete[] code; }
//	void swap(ShaderCode& c) { std::swap(c.code, code); }
//};

using ShaderCode = std::unique_ptr<char[]>;

class ShaderCodeVector : protected std::vector < ShaderCode > {
public:
	void push_copy(const char *code, size_t len) {
		char * c = new char[len + 1];
		c[len] = 0;
		memcpy(c, code, len);
		push_back(ShaderCode());
		back().reset(c);
	}
	void push_copy_front(const char *code, size_t len) {
		ShaderCode sc;
		sc.reset(new char[len + 1]);
		sc[len] = 0;
		memcpy(sc.get(), code, len);
		//emplace(begin() + m_FrontLocation, std::move(sc));
		insert(begin() + m_FrontLocation, ShaderCode());
		at(m_FrontLocation).swap(sc);
		++m_FrontLocation;
	}

	void push_copy_at_front(const char *code, size_t len) {
		ShaderCode sc;
		sc.reset(new char[len + 1]);
		sc[len] = 0;
		memcpy(sc.get(), code, len);
		//emplace(begin() + m_FrontLocation, std::move(sc));
		insert(begin(), std::move(sc));
	}

	string Linear() const {
		std::stringstream ss;
		for (auto &it : *this)
			ss << it.get() << "\n\n//--=next-file=--\n\n";
		return ss.str();
	}
	const char **get() const {
		if (size() == 0) return nullptr;
		return (const char**)&((*this)[0]);
	}
	size_t len() const { return size(); }

	typedef std::vector < ShaderCode > BaseClass;
	using BaseClass::begin;
	using BaseClass::end;
	using BaseClass::iterator;
private:
	size_t m_FrontLocation = 0;
};

} // namespace Shaders 
} // namespace Graphic 
