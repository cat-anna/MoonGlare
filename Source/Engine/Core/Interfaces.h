/*
 * Interfaces.h
 *
 *  Created on: 03-11-2013
 *      Author: Paweu
 */
#ifndef INTERFACESH
#define INTERFACES_H
namespace Core {

class Interfaces {
public:
	enum class ClassLists {
		Core, 
		Resource,
		
		MaxValue,
	};

	template<class BASE>
	static BASE* CreateCoreClass(const string& ClassName) {
		return CreateClassRaw<BASE, ClassLists::Core>(ClassName);
	}

	template<class BASE>
	static BASE* CreateResourceClass(const string& ClassName) {
		return CreateClassRaw<BASE, ClassLists::Resource>(ClassName);
	}

	static void DumpLists(ostream &out);

	template<class REQUIRE, ClassLists LIST>
	struct sClassRegRoot {
		sClassRegRoot(const char*Name, const GabiLib::GabiTypeInfo * ClassInfo) {
			RegisterClass(Name, ClassInfo, REQUIRE::GetStaticTypeInfo(), LIST);
		}
	};
	template<class REQUIRE, ClassLists LIST>
	struct AutoClassRegRoot {
		AutoClassRegRoot(const char*Name, const GabiLib::GabiTypeInfo * ClassInfo) {
			AutoRegisterClass(Name, ClassInfo, REQUIRE::GetStaticTypeInfo(), LIST);
		}
	};

#define INTERFACE_GEN_CLASS_REG(NAME, REQ, LIST)														\
	template <class T>																					\
	struct Auto##NAME : public AutoClassRegRoot<REQ, ClassLists::LIST> {								\
		Auto##NAME(const char* Name) : AutoClassRegRoot<REQ, ClassLists::LIST>(Name, T::GetStaticTypeInfo()) { }\
		Auto##NAME() : AutoClassRegRoot<REQ, ClassLists::LIST>(0, T::GetStaticTypeInfo()) { }\
	};
	INTERFACE_GEN_CLASS_REG(CoreClassReg, cRootClass, Core)
	INTERFACE_GEN_CLASS_REG(ResourceClassReg, DataClasses::DataClass, Resource)
#undef INTERFACE_GEN_CLASS_REG

	static void Initialize();

	struct ClassInfo {
		string Name;
		const GabiLib::GabiTypeInfo *Class;
	};

	static void EnumerateClassLists(std::function<void(ClassLists listID, const string& ListName)> func);
	static void EnumerateClassListContent(ClassLists listID, std::function<void(const ClassInfo& info)> func);
	static void RegisterScriptApi(ApiInitializer &api);
private:
	typedef std::unordered_map<string, ClassInfo> ClassList;
	struct InterfaceList {
		string Name;
		ClassList List;
		void Add(const char* pName, const GabiLib::GabiTypeInfo *type) {
			ClassInfo info;
			info.Class = type;
			if (!pName) pName = type->Name;
			info.Name = pName;
			List.insert(std::make_pair(pName, info));
		}
		const GabiLib::GabiTypeInfo *GetClass(const string &name) const  {
			auto it = List.find(name);
			if (it == List.end()) return 0;
			return it->second.Class;
		}
		GabiLib::GabiObject *CreateClass(const string &name) const {
			auto Class = GetClass(name);
			if (!Class || !Class->NewObject) return 0;
			return Class->NewObject();
		}
	};
	class cClassListVector : protected std::vector<InterfaceList> {
	public:
		typedef std::vector<InterfaceList> BaseClass;
		cClassListVector(unsigned Size) : BaseClass(Size) {}
		InterfaceList& operator[](unsigned index) { return at(index); }
		string& Name(unsigned index) { return at(index).Name; };
		using BaseClass::size;
		using BaseClass::begin;
		using BaseClass::end;
		typedef BaseClass::iterator iterator;
	};
	cClassListVector m_ClassLists;
	Interfaces();

	template <class CAST, ClassLists List, class BASE = CAST>
	static CAST* CreateClassRaw(const string& ClassName) {
		return (CAST*)CreateClass(ClassName, List, BASE::GetStaticTypeInfo());
	}

	static void RegisterClass(const char *Name, const GabiLib::GabiTypeInfo *ClassInfo, const GabiLib::GabiTypeInfo *Requirement, ClassLists List);
	static void AutoRegisterClass(const char *Name, const GabiLib::GabiTypeInfo *ClassInfo, const GabiLib::GabiTypeInfo *Requirement, ClassLists List);
	static cRootClass *CreateClass(const string& ClassName, ClassLists List, const GabiLib::GabiTypeInfo *Requirement);
	static void DumpList(ClassList &list, ostream &out);
	static Interfaces *Instance;
};

} //namespace Core
#endif // CINTERFACESLIST_H_ 
