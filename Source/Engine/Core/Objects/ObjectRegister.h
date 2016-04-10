/*
 * cObjectList.h
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#ifndef COBJECTLIST_H_
#define COBJECTLIST_H_
namespace Core {
namespace Objects {

class ObjectRegister : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(ObjectRegister, cRootClass)
public:
	template<class ... ARGS>
	using Allocator_t = Space::Memory::StaticMultiAllocator<Configuration::Storage::Static::ObjectBuffer, ARGS...>;
	using Generations_t = Space::Memory::GenerationRandomAllocator<Allocator_t, Handle>;

	ObjectRegister();
	~ObjectRegister();

	bool InitializeObjects() {
		bool ret = true;
		for (auto &it : m_ActiveObjects)
			ret &= it->Initialize();
		return ret;
	}
	bool FinalizeObjects() {
		bool ret = true;
		for (auto &it : m_ActiveObjects)
			ret &= it->Finalize();
		return ret;
	}

	using iterator = std::vector<Object*>::iterator;
	iterator begin() { return m_ActiveObjects.begin(); }
	iterator end() { return m_ActiveObjects.end(); }

	size_t size() const { return m_Objects.size(); }

	Handle NewObject() { return Allocate().second; }
	Object* NewObjectByPtr() { return Allocate().first; }

	Handle Insert(Object* obj);
	Handle Insert(std::unique_ptr<Object> obj);
	void Release(Handle h);
	void Remove(Handle h);
	Object *Get(Handle h);

	Handle FindByName(const string& Name) {
		//auto &list = m_NameMap[Name];
		//if (list.empty()) return 0;
		//return list.front();
	}

	HandleSet FindAllByName(const string& Name) {
		//auto &list = m_NameMap[Name];
		//if (list.empty()) return 0;
		//return list.front();
	}

	//const ObjectList& GetObjectsByType(const string& Type) {
	//	return m_TypeMap[Type];
	//}

//	void Remove(Object* object) { 
//		if (!object) {
//			AddLog(Error, "Attempt to remove null object from register!");
//			return;
//		}
//		m_List.remove(object); 
//		m_NameMap[object->GetName()].remove(object);
//		m_TypeMap[object->GetPatternName()].remove(object);
//	}
//
//	void Add(Object* object) { 
//		if (!object) {
//			AddLog(Error, "Attempt to add null object to register!");
//			return;
//		}
//		m_List.push_back(object); 
//		m_NameMap[object->GetName()].push_back(object);
//		m_TypeMap[object->GetPatternName()].push_back(object);
//	}

	void Clear();
	static void RegisterScriptApi(ApiInitializer &api);

	bool LoadObjects(const xml_node SrcNode, GameScene *OwnerScene);
protected:
	Generations_t m_Generations;
	std::vector<std::unique_ptr<Object>> m_Objects;
	std::vector<Object*> m_ActiveObjects;

	std::pair<Object*, Handle> Allocate();
};

} // namespace Objects
} // namespace Core
#endif // COBJECTLIST_H_ 
