#ifndef SPACE_RTTI_RTTIH
#define SPACE_RTTI_RTTIH

#include "../Configuration.h"

//---------------------- PARTIAL DECLARATORS ------------------------------------

#ifndef SPACERTTI_TRACK_INSTANCES
#define SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)
#else
#define SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)																\
		void* operator new(size_t t) { ThisClass::GetStaticTypeInfo()->NewInstance(); return malloc(t); }		\
		void* operator new(size_t, void *p) { ThisClass::GetStaticTypeInfo()->NewInstance(); return p; }		\
		void  operator delete(void* p) { ThisClass::GetStaticTypeInfo()->DeleteInstance(); free(p); }
#endif

#define SPACERTTI_DECLARE_TYPE_GET_STATIC(NAME, BASE)\
		static const Space::RTTI::TypeInfo* GetStaticTypeInfo  () { return &m_TypeInfo; }

#define SPACERTTI_DECLARE_TYPE_GET_DYNAMIC(NAME, BASE)\
		virtual const Space::RTTI::TypeInfo* GetDynamicTypeInfo () const;

#define SPACERTTI_DECLARE_TYPE_GET_DYNAMIC_ABSTRACT(NAME, BASE)\
		virtual const Space::RTTI::TypeInfo* GetDynamicTypeInfo () const = 0;

#define SPACERTTI_DECLARE_CREATOR(NAME, BASE)\
		static Space::RTTI::RTTIObject* NewObject();

#define SPACERTTI_DECLARE_TYPE_MEMBER()\
		static const Space::RTTI::TypeInfo m_TypeInfo;

#define SPACERTTI_DECLARE_BASE_CLASS(NAME, BASE) using BaseClass = BASE;
#define SPACERTTI_DECLARE_THIS_CLASS(NAME, BASE) using ThisClass = NAME;

#define SPACERTTI_DECLARE_CLASS_TYPES(NAME, BASE)					\
			SPACERTTI_DECLARE_BASE_CLASS(NAME, BASE)				\
			SPACERTTI_DECLARE_THIS_CLASS(NAME, BASE)

#define SPACERTTI_DECLARE_SINGLETON_INSTANCE(NAME, BASE)\
		static NAME* _Instance;

#define SPACERTTI_DECLARE_SINGLETON_ACCESS(NAME, BASE)\
		static NAME* Instance() { SPACERTTI_SINGLETON_CHECK; return _Instance;}	\
		static void DeleteInstance() { delete _Instance; _Instance = nullptr; }	\
		static void ReleaseInstance() { _Instance = nullptr; }	\
		static bool InstanceExists() { return _Instance != nullptr; }

#define SPACERTTI_DECLARE_SINGLETON_SETTER(NAME, BASE)\
		void SetThisAsInstance() { SPACERTTI_SINGLETON_SET_CHECK; _Instance = this; }

//---------------------- PARTIAL ? ------------------------------------

#define SPACERTTI_DECLARE_NO_COPY_CONSTRUCTOR(NAME, BASE)\
		NAME(const NAME&) = delete;

#define SPACERTTI_DECLARE_NO_ASSIGNMENT(NAME, BASE)\
		NAME& operator=(const NAME&) = delete;

//---------------------- PARTIAL IMPLEMENTATORS ------------------------------------

#define SPACERTTI_IMPLEMENT_CREATOR(NAME)\
		Space::RTTI::RTTIObject* NAME::NewObject() { return new NAME; }

#define SPACERTTI_IMPLEMENT_GET_DYNAMIC(NAME)\
		const Space::RTTI::TypeInfo* NAME::GetDynamicTypeInfo () const { return &NAME::m_TypeInfo; }

#define SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO(NAME)\
		const Space::RTTI::TypeInfo  NAME::m_TypeInfo (#NAME, typeid(NAME).name(), NAME::BaseClass::GetStaticTypeInfo(), sizeof(NAME), &NAME::NewObject, false, false);
														
#define SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_ABSTRACT(NAME)\
		const Space::RTTI::TypeInfo  NAME::m_TypeInfo (#NAME, typeid(NAME).name(), NAME::BaseClass::GetStaticTypeInfo(), sizeof(NAME), 0, true, false);
														
#define SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_NOCREATOR(NAME)\
		const Space::RTTI::TypeInfo  NAME::m_TypeInfo (#NAME, typeid(NAME).name(), NAME::BaseClass::GetStaticTypeInfo(), sizeof(NAME), 0, false, false);
														
#define SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_SINGLETON(NAME)\
		const Space::RTTI::TypeInfo  NAME::m_TypeInfo (#NAME, typeid(NAME).name(), NAME::BaseClass::GetStaticTypeInfo(), sizeof(NAME), 0, false, true);
														
#define SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_DYNAMIC_SINGLETON(NAME)\
		const Space::RTTI::TypeInfo  NAME::m_TypeInfo (#NAME, typeid(NAME).name(), NAME::BaseClass::GetStaticTypeInfo(), sizeof(NAME), &NAME::NewObject, false, true);

#define SPACERTTI_IMPLEMENT_SINGLETON_INSTANCE(NAME)\
		NAME* NAME::_Instance = nullptr;

//----------------------- BASIC MODE ------------------------------------

#define SPACERTTI_DECLARE_CLASS(NAME, BASE)								\
	private:														\
		SPACERTTI_DECLARE_TYPE_MEMBER()						\
	public:															\
		SPACERTTI_DECLARE_TYPE_GET_STATIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_TYPE_GET_DYNAMIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_CREATOR(NAME, BASE)	 						\
		SPACERTTI_DECLARE_CLASS_TYPES(NAME, BASE)						\
		SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)

#define SPACERTTI_IMPLEMENT_CLASS(NAME)									\
	SPACERTTI_IMPLEMENT_CREATOR(NAME)									\
	SPACERTTI_IMPLEMENT_GET_DYNAMIC(NAME)								\
	SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO(NAME)

//----------------------- ABSTRACT MODE ------------------------------------

#define SPACERTTI_DECLARE_ABSTRACT_CLASS(NAME, BASE)						\
	private:														\
		SPACERTTI_DECLARE_TYPE_MEMBER()						\
	public:															\
		SPACERTTI_DECLARE_TYPE_GET_STATIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_TYPE_GET_DYNAMIC_ABSTRACT(NAME, BASE) 			\
		SPACERTTI_DECLARE_CLASS_TYPES(NAME, BASE)						\
		SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)

#define SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(NAME)\
	SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_ABSTRACT(NAME)

//----------------------- BASIC NO CREATOR ------------------------------------

#define SPACERTTI_DECLARE_CLASS_NOCREATOR(NAME, BASE)					\
	private:														\
		SPACERTTI_DECLARE_TYPE_MEMBER()						\
	public:															\
		SPACERTTI_DECLARE_TYPE_GET_STATIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_TYPE_GET_DYNAMIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_CLASS_TYPES(NAME, BASE)						\
		SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)

#define SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(NAME)						\
		SPACERTTI_IMPLEMENT_GET_DYNAMIC(NAME)							\
		SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_NOCREATOR(NAME)

#define SPACERTTI_DECLARE_STATIC_CLASS(NAME, BASE)	SPACERTTI_DECLARE_CLASS_NOCREATOR(NAME, BASE)
#define SPACERTTI_IMPLEMENT_STATIC_CLASS(NAME)		SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(NAME)

//----------------------- BASIC SINGLETON ------------------------------------

#define SPACERTTI_DECLARE_CLASS_SINGLETON(NAME, BASE)					\
	private:														\
		SPACERTTI_DECLARE_TYPE_MEMBER()						\
		SPACERTTI_DECLARE_SINGLETON_INSTANCE(NAME, BASE)					\
		SPACERTTI_DECLARE_NO_COPY_CONSTRUCTOR(NAME, BASE)				\
		SPACERTTI_DECLARE_NO_ASSIGNMENT(NAME, BASE)						\
	protected:														\
		SPACERTTI_DECLARE_SINGLETON_SETTER(NAME, BASE)					\
	public:															\
		SPACERTTI_DECLARE_TYPE_GET_STATIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_TYPE_GET_DYNAMIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_CLASS_TYPES(NAME, BASE)						\
		SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)					\
		SPACERTTI_DECLARE_SINGLETON_ACCESS(NAME, BASE)					

#define SPACERTTI_IMPLEMENT_CLASS_SINGLETON(NAME)						\
		SPACERTTI_IMPLEMENT_GET_DYNAMIC(NAME)							\
		SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_SINGLETON(NAME)				\
		SPACERTTI_IMPLEMENT_SINGLETON_INSTANCE(NAME)

//----------------------- DYNAMIC SINGLETON ------------------------------------

#define SPACERTTI_DECLARE_CLASS_DYNAMIC_SINGLETON(NAME, BASE)			\
	private:														\
		SPACERTTI_DECLARE_TYPE_MEMBER(NAME, BASE)						\
		SPACERTTI_DECLARE_SINGLETON_INSTANCE(NAME, BASE)					\
		SPACERTTI_DECLARE_NO_COPY_CONSTRUCTOR(NAME, BASE)				\
		SPACERTTI_DECLARE_NO_ASSIGNMENT(NAME, BASE)						\
	protected:														\
		SPACERTTI_DECLARE_SINGLETON_SETTER(NAME, BASE)					\
	public:															\
		SPACERTTI_DECLARE_CREATOR(NAME, BASE)	 						\
		SPACERTTI_DECLARE_TYPE_GET_STATIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_TYPE_GET_DYNAMIC(NAME, BASE) 					\
		SPACERTTI_DECLARE_CLASS_TYPES(NAME, BASE)						\
		SPACERTTI_DECLARE_DEBUG_OPERATORS(NAME, BASE)					\
		SPACERTTI_DECLARE_SINGLETON_ACCESS(NAME, BASE)					

#define SPACERTTI_IMPLEMENT_CLASS_DYNAMIC_SINGLETON(NAME)				\
		SPACERTTI_IMPLEMENT_CREATOR(NAME)								\
		SPACERTTI_IMPLEMENT_GET_DYNAMIC(NAME)							\
		SPACERTTI_IMPLEMENT_BUILD_TYPE_INFO_DYNAMIC_SINGLETON(NAME)		\
		SPACERTTI_IMPLEMENT_SINGLETON_INSTANCE(NAME)

//----------------------- PARTIAL RTTI ------------------------------------

#define SPACERTTI_DECLARE_CLASS_NOBASE(NAME) 							\
	public:															\
		SPACERTTI_DECLARE_THIS_CLASS(NAME, void*)						\

//------------------------------------------------------------------------

namespace Space {
namespace RTTI {

struct TypeInfo;
class RTTIObject;
typedef RTTIObject*(*ConstructorFunPtr)();

class Tracker {
public:
	typedef std::list<const TypeInfo*> TypeList;
	typedef TypeList::const_iterator iterator;

	static size_t Count() { ForceList(); return _List->size(); }
	static iterator begin() { ForceList(); return _List->begin(); }
	static iterator end() { ForceList(); return _List->end(); }

	static void DumpClasees(std::ostream &out = std::cout);
	static void DumpInstances(std::ostream &out = std::cout);

	static void SortList();
private:
	friend struct TypeInfo;
	static TypeList *_List;
	static void InsertClass(const TypeInfo * Class);
	static void ForceList() { if (!_List) _List = new TypeList(); }
};

struct TypeInfo {
	TypeInfo(const char* const TypeName, const char* const TypeIdName, const TypeInfo* BaseType, unsigned Size, ConstructorFunPtr NewObjFn, bool Abstract, bool Singleton) :
		m_Name(TypeName),
		m_Base(BaseType),
		m_NewObject(NewObjFn),
		m_TypeSize(Size),
		InstancesCount(0),
		DeletedCount(0) {

		m_Flags.m_U32Value = 0;
		if (!NewObjFn)
			m_Flags.m_Static = true;
		m_Flags.m_Abstract = Abstract;
		m_Flags.m_Singleton = Singleton;

		Space::RTTI::Tracker::InsertClass(this);
	}

	template<class T>
	TypeInfo(T t) {
		m_TypeSize = T::TypeSize;

		m_Flags.m_U32Value = 0;
		m_Flags.m_Abstract = T::IsAbstract::value;
		m_Flags.m_POD = T::IsPod::value;
		m_Flags.m_Static = true;

		m_Name = T::GetClassName();

		m_Base = nullptr;
		m_NewObject = nullptr;

		Space::RTTI::Tracker::InsertClass(this);
	}

	union {
		struct {
			bool m_Abstract : 1;
			bool m_Static : 1;
			bool m_Singleton : 1;
				 
			bool m_POD : 1;
		};
		uint32_t m_U32Value;
	} m_Flags;

	uint32_t m_TypeSize;

	const char* GetName() const { return m_Name; }

	const char* m_Name;
	const TypeInfo* m_Base;
	ConstructorFunPtr m_NewObject;

	bool IsAbstract() const { return m_Flags.m_Abstract; }

	mutable int InstancesCount;
	mutable int DeletedCount;

	void NewInstance() const { ++InstancesCount; }
	void DeleteInstance() const {
		++DeletedCount;
		--InstancesCount;
	}
	
	//                                         1 2     3   4    5   6   7   8
	static constexpr char *PrintLineFormat = "%s%-80s %4s %12s %7s %8s %9s %3s";
}; 

template<class T>
struct TemplateTypeInfo {
	static const TypeInfo s_TypeInfo;
	static const TypeInfo* GetStaticTypeInfo() { return &s_TypeInfo; }

	using ThisClass = T;
};

template<class T> const TypeInfo TemplateTypeInfo<T>::s_TypeInfo = TypeInfo(TemplateTypeInfoData<T>());
template<class T> const TypeInfo* GetStaticTypeInfo() { return &TemplateTypeInfo<T>::s_TypeInfo; }

template<class T>
struct TemplateTypeInfoData {
	using Type_t = T;											

	constexpr static const char* GetRawClassName() { return typeid(T).name(); }
	static const char* GetClassName() { 
		auto name = typeid(T).name();
		if (strncmp(name, "class", 5) == 0) {
			return name + 6;
		}
		if (strncmp(name, "struct", 6) == 0) {
			return name + 7;
		}
		if (strncmp(name, "union", 5) == 0) {
			return name + 6;
		}
		return name;
	}
																			
	using IsPod = typename std::is_pod<Type_t>::type;						
	using IsAbstract = typename std::is_abstract<Type_t>::type;

	enum {
		TypeSize = sizeof(Type_t),
	};			

	const char *m_ClassAlias = nullptr;
	const char* GetClassAlias() { return m_ClassAlias ? m_ClassAlias : typeid(T).name(); }

	const TypeInfo* GetTypeInfo() const { return GetStaticTypeInfo<Type_t>(); }
};

template<class ... ARGS>
struct TypeInfoInitializer {
	TypeInfoInitializer() {
		const TypeInfo* v[] = { GetStaticTypeInfo<ARGS>()... };
		(void) v;
	}
};


class RTTIObject {
public:
	typedef RTTIObject ThisClass;
	typedef void BaseClass;

	static  const TypeInfo* GetStaticTypeInfo()  { return &m_TypeInfo; }
	virtual const TypeInfo* GetDynamicTypeInfo() const = 0;

	RTTIObject();
	virtual ~RTTIObject();

	static bool IsClassInstance(const RTTIObject* Obj, const TypeInfo* TypeInfo);
	static bool IsBaseClass(const TypeInfo* BaseType, const TypeInfo* InstanceType);
	bool IsInstanceOf(const TypeInfo* TypeInfo);
private:
	static const TypeInfo m_TypeInfo;
};
 
} //namespace RTTI
} //namespace Space

inline std::ostream& operator << (std::ostream &out, const Space::RTTI::TypeInfo &ti) {

	const char* usedstr = (ti.InstancesCount + ti.DeletedCount ? "Yes" : "");
	unsigned align16 = ti.m_TypeSize % 16;

	char sizebuf[32];
	sprintf_s(sizebuf, "%8d[%2d]", ti.m_TypeSize, align16);

	char buf[1024];
	sprintf_s(buf, Space::RTTI::TypeInfo::PrintLineFormat,
		/* 1 */ "",
		/* 2 */ ti.GetName(),
		/* 3 */ usedstr,
		/* 4 */ sizebuf,
		/* 5 */ (!(ti.m_Flags.m_Static) ? "   Y " : ""),
		/* 6 */ (ti.m_Flags.m_Abstract ? "   Y " : ""),
		/* 7 */ (ti.m_Flags.m_Singleton ? "   Y " : ""),
		/* 8 */ (ti.m_Flags.m_POD ? " Y " : "")
	);
	return out << buf;
}

#endif
