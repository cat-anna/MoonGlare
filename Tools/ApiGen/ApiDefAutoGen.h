#ifndef APIDEFAUTOGEN_H_
#define APIDEFAUTOGEN_H_

#include <pugixml-1.2/src/pugixml.hpp>
//#include <cxxabi.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <map>
#include <vector>
#include <queue>
#include <type_traits>

using pugi::xml_node;

namespace ApiDefAutoGen {

using StringVector = std::vector < std::string > ;

template <class R, class T>
R inline UnionCast(T t){
	union{
		T t;
		R r;
	} u;
	u.t = t;
	return u.r;
}

inline xml_node XML_NamedChild(xml_node parent, const char* NodeName, const char* Name = 0) {
	xml_node node = parent.find_child_by_attribute(NodeName, "Name", Name);
	if (node) return node;
	node = parent.append_child(NodeName);
	if(Name)
		node.append_attribute("Name") = Name;
	return node;
}

class Class;

class Namespace {
protected:
	Namespace *m_parent = 0;
	xml_node m_root;
	Namespace(xml_node root):m_parent(0), m_root(root) {}
	Namespace(xml_node root, Namespace *parent):m_parent(parent), m_root(root) {}

public:
	Namespace():m_parent(), m_root(){};
	Namespace(const Namespace & other): m_parent(other.m_parent), m_root(other.m_root) {}
	Namespace(Namespace && other): m_parent(other.m_parent), m_root(other.m_root) {
		other.m_parent = &other;
		other.m_root = xml_node();
	}

	Namespace beginNamespace(const char* name);
	Namespace &endNamespace();

	virtual ~Namespace();

	template<class T> Namespace &addPtrVariable(const char* name, T *t);
	template<class T> Namespace &addVariable(const char* name, T *t, bool Writable = true);

	template<class T> 
	Namespace &addProperty(const char* name, T (*get)(), void(*set)(T) = nullptr){
		T *t = 0;
		return addVariable(name, t, set != nullptr);
	}

	template<class T> Class beginClass(const char *name);
	template<class T, class Base> Class deriveClass(const char *name);

	template<class T> Namespace &addFunction(const char *name, T t);
	Namespace &addCFunction(const char *name, int(*)(lua_State*));

	static Namespace Begin(const char* name = (const char*)0);
	static void Initialize(const char* fn);
	static void SetRoot(const char* name);
	static void Finalize();
	static std::string TranslateName(const char* name);
protected:
	static bool m_initialized;
	static pugi::xml_document xml_doc;
	static const char* m_OutFile;
	static const char* m_xmlRoot;
	typedef std::map<std::string, std::string> StringMap;
	static StringMap m_KnownClasees;
};

class Class : protected Namespace {
protected:
	Class(Namespace *parent, xml_node root, const char *name);
	Class(xml_node root, const char *name, Namespace *parent);
public:
	Class(const Class & other){
		m_root = other.m_root;
		m_parent = other.m_parent;
	}
	Class(Class && other) {
		m_root = other.m_root;
		m_parent = other.m_parent;
		other.m_parent = &other;
		other.m_root = xml_node();
	}

	template<class T, class Base> static Class Begin(Namespace *parent, xml_node root, const char *name);
	template<class T> static Class Begin(Namespace *parent, xml_node root, const char *name);

	template<class T> Class &addFunction(const char *name, T t);

	template <class T, class OWNER> Class& addProperty (char const* name, T (OWNER::*get) () const, void (OWNER::*set) (T));
	template <class T, class OWNER> Class& addProperty (char const* name, T (OWNER::*get) () const);
	template <class T, class OWNER> Class& addData (char const* name, T OWNER::*ptr, bool Writable = true);

	template <class T> Class& addConstructor();

	template <class OWNER> 
	Class& addCFunction(const char *name, int(OWNER::*ptr)(lua_State*)) {
	//	Namespace::addCFunction(name, (int( *)(lua_State*))ptr);
		return *this;
	}

	Namespace &endClass();
};

//------------------------------------------------------------------------

struct sTypeName {
	bool IsPtr;
	bool IsConst;
	std::string Name;
};

template<class T>
sTypeName GetTypeOf() {
	//int status;
	sTypeName t;
	t.Name = Namespace::TranslateName(typeid(T).name());
		//abi::__cxa_demangle(typeid(T).name(), 0, 0, &status));
	t.IsPtr = std::is_pointer<T>::value;
	t.IsConst = std::is_const<T>::value;
	return t;
}

template<class T>
inline std::string NameOf() {
	//int status;
	return Namespace::TranslateName(typeid(T).name());
		//abi::__cxa_demangle(typeid(T).name(), 0, 0, &status));
}

void WriteParams(xml_node node, const StringVector *Names, int paramid);

template <class ...Types>
void WriteParams(xml_node item, const StringVector *Names, int paramid, sTypeName ParamType, Types... args){
	xml_node param = item.append_child("Parameter");
	param.append_attribute("id") = paramid;
	param.append_attribute("Type") = ParamType.Name.c_str();
	if(Names && (int)Names->size() > paramid)
		param.append_attribute("Name") = Names->at(paramid).c_str();
	if(ParamType.IsPtr)		param.append_attribute("IsPointer") = true;
	if(ParamType.IsConst)	param.append_attribute("Const") = true;		
	WriteParams(item, Names, ++paramid, args...);
}

template<class ...Types>
static void UnpackParams(xml_node item, const StringVector *Names = 0){
	WriteParams(item, Names, 0, GetTypeOf<Types>()...);
}

struct FunctionHelper {
	template<class RET, class ...Types>
	static void Unpack(xml_node node, RET(*)(Types...)){
		node.append_attribute("Return") = NameOf<RET>().c_str();
		UnpackParams<Types...>(node);
	}
	
	template<class RET, class OWNER, class ...Types>
	static void Unpack(xml_node node, RET(OWNER::*)(Types...)){
		Unpack(node, UnionCast<RET(*)(Types...)>(0));
	}	
	
	template<class RET, class OWNER, class ...Types>
	static void Unpack(xml_node node, RET(OWNER::*)(Types...)const){
		node.append_attribute("Const") = true;
		Unpack(node, UnionCast<RET(OWNER::*)(Types...)>(0));
	}	
};

//------------------------------------------------------------------------

template<class T>
Class Namespace::beginClass(const char *name){
	return Class::Begin<T>(this, m_root, name);
}

template<class T, class Base>
Class Namespace::deriveClass(const char *name){
	return Class::Begin<T, Base>(this, m_root, name);
}

template<class T>
Namespace& Namespace::addVariable(const char* name, T *t, bool Writable) {
	xml_node node = m_root.append_child("Variable");;
	node.append_attribute("Name") = name;
	node.append_attribute("Type") = NameOf<T>().c_str();
	if (!Writable)
		node.append_attribute("Const") = true;
	return *this;
}

template<class T>
Namespace &Namespace::addPtrVariable(const char* name, T *t) {
	xml_node node = m_root.append_child("Variable");
	node.append_attribute("Name") = name;
	node.append_attribute("Type") = NameOf<T>().c_str();
	node.append_attribute("Const") = true;
	return *this;
}

template<class T>
Namespace& Namespace::addFunction(const char *name, T t){
	xml_node f = XML_NamedChild(m_root, "Function", name);
	FunctionHelper::Unpack(f, t);
	return *this;
}

//----------------------------------------------------

template<class T, class Base>
Class Class::Begin(Namespace *parent, xml_node root, const char *name){
	m_KnownClasees[NameOf<T>()] = name;
	Class c(parent, root, name);
	XML_NamedChild(c.m_root, "Derive", NameOf<Base>().c_str());
	return c;
}

template<class T>
Class Class::Begin(Namespace *parent, xml_node root, const char *name){
	m_KnownClasees[NameOf<T>()] = name;
	return Class(parent, root, name);
}

template<class T>
Class& Class::addFunction(const char *name, T t){
	xml_node f = XML_NamedChild(m_root, "Function", name);
	FunctionHelper::Unpack(f, t);
	return *this;
}

template <class T, class OWNER>
Class& Class::addProperty (char const* name, T (OWNER::*get) () const, void (OWNER::*set) (T)){
	xml_node f = XML_NamedChild(m_root, "Property", name);
	f.append_attribute("Type") = NameOf<T>().c_str();
	return *this;
}

template <class T, class OWNER>
Class& Class::addProperty (char const* name, T (OWNER::*get) () const){
	xml_node f = XML_NamedChild(m_root, "Property", name);
	f.append_attribute("Type") = NameOf<T>().c_str();
	f.append_attribute("Read-only") = true;
	return *this;
}

template <class T, class OWNER>
Class& Class::addData (char const* name, T OWNER::*ptr, bool Writable){
	xml_node node;
	if (Writable) node = m_root.append_child("Variable");
	else node = m_root.append_child("Const");
	node.append_attribute("Name") = name;
	node.append_attribute("Type") = NameOf<T>().c_str();
	return *this;
}

template <class T>
Class& Class::addConstructor(){
	FunctionHelper::Unpack(XML_NamedChild(m_root, "Constructor"), T(0));
	return *this;
}

//-----------------------------------------------------------------------------------------------
struct MakeFunRoot;

class OutCallsPicker {
public:
	static void RegisterDefinition(MakeFunRoot *ptr);
	static void WriteOutCalls(const char* filename);
	template<class RET, class NAME, class CALLER, class ...Types>	static void AddDefinition();
private:
	static void Initialize();
	static std::vector<MakeFunRoot*> *m_List;
	static pugi::xml_document *xml_doc;
	static bool m_initialized;
};

struct MakeFunRoot {
	MakeFunRoot() { OutCallsPicker::RegisterDefinition(this); }
	virtual ~MakeFunRoot() {};
	virtual void WriteOutCall() = 0;
	virtual void dummy() = 0;
};

template <class RET, class NAME, class CALLER, class ...Types>
struct MakeFunCaller : public MakeFunRoot {
	MakeFunCaller(): MakeFunRoot() {}
	void WriteOutCall(){ OutCallsPicker::AddDefinition<RET, NAME, CALLER, Types...>(); }
	void dummy() {};
};

template <class RET, class NAME, class CALLER, class ...Types>
struct MakeFunHelper {
	static MakeFunRoot *dummy;
};

template <class RET, class NAME, class CALLER, class ...Types>
MakeFunRoot* MakeFunHelper<RET, NAME, CALLER, Types...>::dummy = 
		new MakeFunCaller<RET, NAME, CALLER, Types...>();

template <class RET, class NAME, class CALLER, class ...Types>
void InsertOutCall(Types ...args){
	MakeFunHelper<RET, NAME, CALLER, Types...>::dummy->dummy();
}

template<class RET, class NAME, class CALLER, class ...Types>
void OutCallsPicker::AddDefinition(){
	xml_node node;
	if (!m_initialized) Initialize();
	node = xml_doc->document_element();
	node = node.append_child("Function");
	node.append_attribute("Name") = NAME().get();
	StringVector sv;
	std::string str = NAME().getP();
	char *t = (char*)str.c_str();
	char* next;
	while(t){
		next = strchr(t, ',');
		if(next){
			*next = 0;
			next += 2;
		}
		sv.push_back(t);
		t = next;
	}
	node.append_attribute("Return") = NameOf<RET>().c_str();
	node.append_attribute("Caller") = NameOf<CALLER>().c_str();
	UnpackParams<Types...>(node, &sv);
}

#define API_GEN_MAKE_DECL(F, CALLER, ...)															\
		struct __##F##__s { 																		\
			const char * get(){ return #F; };  														\
			const char * getP(){ return #__VA_ARGS__; };  };										\
		ApiDefAutoGen::InsertOutCall<int, __##F##__s, decltype(CALLER)>(__VA_ARGS__);

}//namespace

#endif /* LADFAUTOGEN_H_ */
