#pragma once
#ifndef XMLUTILSH
#define XMLUTILSH

static const char *XYZ_Captions[] ={ "X", "Y", "Z", 0, };

inline pugi::xml_node CheckXML(pugi::xml_node Parent, const char *Name, pugi::xml_node dst = pugi::xml_node());

inline pugi::xml_node CheckXML(pugi::xml_node Parent, const char *Name, pugi::xml_node dst) {
	if (dst)
		return dst;
	dst = Parent.child(Name);
	if (!dst)
		dst = Parent.append_child(Name);
	return dst;
}
#ifdef XMATH_H
inline bool operator <<(pugi::xml_node Node, math::vec3 &out) {
	if (!Node) return false;
	for (unsigned i = 0; i < 3; ++i)
		Node.append_attribute(XYZ_Captions[i]) = out[i];
	return true;
};
#endif

namespace XML {

typedef const char *xml_string;

namespace Captions {
	static const xml_string XYZ[]  ={ "X", "Y", "Z", 0, };
	static const xml_string UVW[]  ={ "U", "V", "W", 0, };
	static const xml_string RGBA[] = { "R", "G", "B", "A", 0 };
};

struct Value {
	template<class T> static int Read(const T t, int def = 0) { return t.as_uint(def); }
	template<class T> static unsigned Read(const T t, unsigned def = 0) { return t.as_uint(def); }
	template<class T> static bool Read(const T t, bool def = 0) { return t.as_bool(def); }
	template<class T> static xml_string Read(const T t, xml_string def = 0) { return t.as_string(def); }
	template<class T> static float Read(const T t, float def = 0) { return t.as_float(def); }
	template<class T> static double Read(const T t, double def = 0) { return t.as_double(def); }

	template<class T> static std::string Read(const T t, const std::string& def) { return t.as_string(def.c_str()); }

	template<class T, class V>
	static V Read(const T t, V& def) {
		return Helper<std::is_enum<V>::type, V>::Read<T>(t, def);
	}

	template<class T> static void Write(T t, int v) { t = v; }
	template<class T> static void Write(T t, unsigned v) { t = v; }
	template<class T> static void Write(T t, bool v)  { t = v; }
	template<class T> static void Write(T t, xml_string v) { t = v; }
	template<class T> static void Write(T t, float v)  { t = v; }
	template<class T> static void Write(T t, double v) { t = v; }

	template<class T> static void Write(T t, const std::string& v) { t = v.c_str(); }

	template<class T, class V>
	static void Write(const T t, V& v) {
		return Helper<std::is_enum<V>::type, V>::Write<T>(t, v);
	}
private:
	template<class IsEnum, class V> struct Helper { };

	template<class V> struct Helper<std::true_type, V> { 
		template<class T>
		static V Read(const T t, V& def) {
			using type_t = std::underlying_type<V>::type;
			return static_cast<V>(Value::Read(t, static_cast<type_t>(def)));
		}
		template<class T>
		static void Write(const T t, V& v) {
			using type_t = std::underlying_type<V>::type;
			return Value::Write(t, static_cast<type_t>(v));
		}
	};

	template<class V> struct Helper<std::false_type, V> { 
		template<class T>
		static V Read(const T t, V& def) {
			return Value::Read(t, def);
		}
		template<class T>
		static void Write(const T t, V& v) {
			return Value::Write(t, v);
		}
	};
	template<> struct Helper<std::false_type, std::string> { 
		template<class T>
		static std::string Read(const T t, std::string& def) {
			return Value::Read(t, def.c_str());
		}
		template<class T>
		static void Write(const T t, std::string& v) {
			return Value::Write(t, v.c_str());
		}
	};
};

struct XML_Common {
protected:
	template <class Value_t, class Vec_t, unsigned Vec_size>
	static bool Read_template(pugi::xml_node parent, xml_string Name, Vec_t &out, const Vec_t &default = Vec_t(), const xml_string captions[] = Captions::XYZ) {
		if (Name && !(parent = parent.child(Name))) {
			out = default;
			return false;
		}
		auto attr = parent.attribute("Value");
		if (attr) {
			Value_t v = Value::Read<>(attr, Value_t(0));
			for (int i = 0; i < Vec_size; ++i)
				out[i] = v;
			return true;
		}
		for (int i = 0; i < Vec_size; ++i)
			out[i] = Value::Read<>(parent.attribute(captions[i]), default[i]);
		return true;
	}
};

struct Node : public XML_Common {
	Node() : m_handle() { };
	Node(pugi::xml_node handle) : m_handle(handle) { };

#ifdef BT_VECTOR3_H
	bool ReadVector(xml_string Name, btVector3 &out, const btVector3 &default = btVector3(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<btScalar, btVector3, 3>(m_handle, Name, out, default, captions);
	}
#endif
#ifdef XMATH_H
	bool ReadVector(xml_string Name, uvec3 &out, const uvec3 &default = uvec3(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<unsigned, uvec3, 3>(m_handle, Name, out, default, captions);
	}
	bool ReadVector(xml_string Name, uvec2 &out, const uvec2 &default = uvec2(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<unsigned, uvec2, 2>(m_handle, Name, out, default, captions);
	}
	bool ReadVector(xml_string Name, math::vec3 &out, const math::vec3 &default = math::vec3(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<float, math::vec3, 3>(m_handle, Name, out, default, captions);
	}
#endif
private:
	pugi::xml_node m_handle;
};

struct Vector : public XML_Common { 
#ifdef BT_VECTOR3_H
	static bool Read(pugi::xml_node parent, xml_string Name, btVector3 &out, const btVector3 &default = btVector3(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<btScalar, btVector3, 3>(parent, Name, out, default, captions);
	}
#endif
#ifdef XMATH_H
	static bool Read(pugi::xml_node parent, xml_string Name, uvec3 &out, const uvec3 &default = uvec3(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<unsigned, uvec3, 3>(parent, Name, out, default, captions);
	}
	static bool Read(pugi::xml_node parent, xml_string Name, uvec2 &out, const uvec2 &default = uvec2(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<unsigned, uvec2, 2>(parent, Name, out, default, captions);
	}

	static bool Read(pugi::xml_node parent, xml_string Name, math::vec3 &out, const math::vec3 &default = math::vec3(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<float, math::vec3, 3>(parent, Name, out, default, captions);
	}
	static bool Read(pugi::xml_node parent, xml_string Name, math::vec2 &out, const math::vec2 &default = math::vec2(), const xml_string captions[] = Captions::XYZ) {
		return Read_template<float, math::vec2, 2>(parent, Name, out, default, captions);
	}
#endif
};

inline bool ReadTextIfPresent(pugi::xml_node parent, xml_string child, std::string& out) {
	parent = parent.child(child);
	if (!parent) return false;
	pugi::xml_text t = parent.text();
	if (!t) return false;
	out = t.as_string();
	return true;
}

inline pugi::xml_node UniqeChild(pugi::xml_node parent, xml_string Name) {
	pugi::xml_node n = parent.child(Name);
	if (n) return n;
	return parent.append_child(Name);
}

inline pugi::xml_attribute UniqeAttrib(pugi::xml_node parent, xml_string Name) {
	auto n = parent.attribute(Name);
	if (n) return n;
	return parent.append_attribute(Name);
}

template <class T>
inline int ForEachChild(pugi::xml_node parent, xml_string Name, T func) {
	for (auto n = parent.child(Name); n; n = n.next_sibling(Name)) {
		int r = func(n);
		if (r != 0)
			return r;
	}
	return 0;
}

template <class T>
bool ReadExistingAttribute(pugi::xml_node parent, xml_string Attr, T& out) {
	if (!parent) return false;
	xml_attribute att = parent.attribute(Attr);
	if (!att) return false;
	out = Value::Read<>(att, T(0));
	return true;
}

template <class T>
bool ReadExistingChildAttribute(pugi::xml_node parent, xml_string Child, xml_string Attr, T& out) {
	if (!parent) return false;
	parent = parent.child(Child);
	if (!parent) return false;
	xml_attribute att = parent.attribute(Attr);
	if (!att) return false;
	out = Value::Read<>(att, T(0));
	return true;
}

#define impl_WriteVector(VEC, LEN, DEF)\
inline void WriteVector(pugi::xml_node parent, xml_string Name, const VEC &vec, const xml_string captions[] = DEF) {\
	parent = UniqeChild(parent, Name);\
	for(int i = 0; i < LEN; ++i) UniqeAttrib(parent, captions[i]) = vec[i];\
}

#define impl_ReadVector(VEC, LEN, READ, DEF)\
inline bool ReadVector(pugi::xml_node parent, xml_string Name, VEC &vec, const VEC& Default = VEC(), const xml_string captions[] = DEF) {\
	if(!(parent = parent.child(Name))) {\
		vec = Default;\
		return false;\
	}\
	auto attr = parent.attribute("Value");\
	if (attr) vec = VEC(attr.READ());\
	else for(int i = 0; i < LEN; ++i) vec[i] = parent.attribute(captions[i]).READ(vec[i]);\
	return true;\
}


#define GEN_VEC_XML_FUN(VEC, LEN, READ, DEF) impl_WriteVector(VEC, LEN, DEF) impl_ReadVector(VEC, LEN, READ, DEF) 
#ifdef XMATH_H
GEN_VEC_XML_FUN(math::fvec2, 2, as_float, Captions::XYZ)
GEN_VEC_XML_FUN(math::fvec3, 3, as_float, Captions::XYZ)
GEN_VEC_XML_FUN(math::uvec2, 2, as_uint, Captions::XYZ)
GEN_VEC_XML_FUN(math::uvec3, 3, as_uint, Captions::XYZ)
#endif

#undef GEN_VEC_XML_FUN
#undef impl_ReadGLMVector
#undef impl_WriteGLMVector

}//namespace XML

#endif
