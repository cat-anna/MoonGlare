/*
 * cObjectList.h
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#ifndef COBJECTLIST_H_
#define COBJECTLIST_H_

namespace MoonGlare {
namespace Core {
namespace Objects {

class ObjectRegister : public cRootClass, public WorldUser {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(ObjectRegister, cRootClass)
public:
	struct Memory {
		template<class T> using Array = std::array<T, Configuration::Storage::Static::ObjectBuffer>;

		template<class ... ARGS>
		using GenerationsAllocator_t = Space::Memory::StaticMultiAllocator<Configuration::Storage::Static::ObjectBuffer, ARGS...>;
		using Generations_t = Space::Memory::GenerationRandomBuffer<GenerationsAllocator_t, Handle>;

		Array<math::mat4> m_GlobalMatrix;
		Array<math::mat4> m_LocalMatrix;
		Array<Handle> m_Parent;
		Array<Handle::Index_t> m_HandleIndex;
		Array<std::unique_ptr<Object>> m_ObjectPtr;
		Generations_t m_HandleAllocator;

		Array<Entity> m_Entity;
	};

	ObjectRegister(World* world);
	~ObjectRegister();

	bool InitializeObjects();
	bool FinalizeObjects();

	using iterator = Memory::Array<std::unique_ptr<Object>>::iterator;
	iterator begin() { return m_Memory->m_ObjectPtr.begin() + 1; }
	iterator end() { return m_Memory->m_ObjectPtr.begin() + m_Memory->m_HandleAllocator.Allocated(); }

	size_t size() const { return m_Memory->m_HandleAllocator.Allocated(); }

	Handle NewObject(Handle Parent); //Not thread-safe!
	Handle NewObject() { return NewObject(GetRootHandle()); }

	Handle Insert(std::unique_ptr<Object> obj, Handle Parent);
	Handle Insert(std::unique_ptr<Object> obj) { return Insert(std::move(obj), GetRootHandle()); }

	void Release(Handle h);
	void Remove(Handle h);
	Object *Get(Handle h);

	Handle GetRootHandle();

	Handle GetParentHandle(Handle h);
	Entity GetParentEntity(Handle h);
	Entity GetEntity(Handle h);

	math::mat4* GetLocalMatrix(Handle h) {
		ASSERT_HANDLE_TYPE(Object, h);
		size_t idx;
		if (!m_Memory->m_HandleAllocator.GetMapping(h, idx))
			return nullptr;
		return &m_Memory->m_LocalMatrix[idx];
	}

	math::mat4* GetGlobalMatrix(Handle h) {
		ASSERT_HANDLE_TYPE(Object, h);
		size_t idx;
		if (!m_Memory->m_HandleAllocator.GetMapping(h, idx))
			return nullptr;
		return &m_Memory->m_GlobalMatrix[idx];
	}

	void Process(const MoveConfig &conf);

	//Handle FindByName(const string& Name) {
		//auto &list = m_NameMap[Name];
		//if (list.empty()) return 0;
		//return list.front();
	//}

	//HandleSet FindAllByName(const string& Name) {
		//auto &list = m_NameMap[Name];
		//if (list.empty()) return 0;
		//return list.front();
	//}

	//const ObjectList& GetObjectsByType(const string& Type) {
	//	return m_TypeMap[Type];
	//}

	void Clear();
	static void RegisterScriptApi(ApiInitializer &api);

	bool LoadObjects(const xml_node SrcNode, GameScene *OwnerScene);
	Handle LoadObject(const std::string &Name, GameScene *OwnerScene, Handle Parent = Handle());
protected:
	std::unique_ptr<Memory> m_Memory;

	void Reorder(size_t start);

	Handle LoadObject(Handle Parent, xml_node MetaXML, GameScene *OwnerScene);
};

} // namespace Objects
} // namespace Core
} //namespace MoonGlare 

#endif // COBJECTLIST_H_ 
