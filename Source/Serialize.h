#pragma once 

//#include "../config.h"

namespace GabiLib {
namespace Serialize {

struct DefaultSetter {
	template<class T, class DEF>
	void Value(T& val, const char* Name, DEF default) {
		val = default;
	}

	template<class T, class Def>
	void ValueContainer(T& val, const char* Name, Def default) {
		val.clear();
		default(val);
	}
	template<class T>
	void Group(T& val, const char* Nam) {
		val.Serialize(*this);
	}
};

#ifdef HEADER_PUGIXML_HPP

struct PugiWritterSerializer {
	PugiWritterSerializer(pugi::xml_node node): m_Node(node) { }

	template<class T, class DEF>
	void Value( T& val, const char* Name, DEF default) {
		XML::Value::Write(m_Node.append_child(Name).text(), val);
	}
	template<class T, class Def>
	void ValueContainer(T& val, const char* Name, Def default) {
		auto child = m_Node.append_child(Name);
		for (auto &it : val) {
			auto node = child.append_child("Item");
			Save(node, it);
		}
	}

	template<class T>
	void Group(T& val, const char* Name) {
		PugiWritterSerializer w(m_Node.append_child(Name));
		val.Serialize(w);
	}
protected:
	template<class T>
	inline bool Save(pugi::xml_node node, const T &s) {
		return s.Save(node);
	}
private:
	pugi::xml_node m_Node;
};

template<>
inline bool PugiWritterSerializer::Save(pugi::xml_node node, const string &s) {
	return node.text() = s.c_str();
}

struct PugiReaderSerializer {
	PugiReaderSerializer(pugi::xml_node node): m_Node(node) { }

	template<class T, class DEF>
	void Value(T& val, const char* Name, DEF default) {
		val = XML::Value::Read(m_Node.child(Name).text(), default);
	}
	template<class T, class Def>
	void ValueContainer(T& val, const char* Name, Def default) {
		val.clear();
		auto child = m_Node.child(Name);
		if (!child) {
			default(val);
			return;
		}
		XML::ForEachChild(child, "Item", [this, &val](pugi::xml_node node) {
			T::value_type t;
			if (Load(node, t))
				val.push_back(t);
			return 0;
		});
	}

	template<class T>
	void Group(T& val, const char* Name) {
		PugiReaderSerializer w(m_Node.child(Name));
		val.Serialize(w);
	}
protected:
	template<class T>
	inline bool Load(const pugi::xml_node node, T &s) {
		return s.Load(node);
	}
private:
	pugi::xml_node m_Node;
};

template<>
inline bool PugiReaderSerializer::Load(const pugi::xml_node node, string &s) {
	s = node.text().as_string();
	return true;
}

#endif //HEADER_PUGIXML_HPP

}
}
