/*
  * Generated by cppsrc.sh
  * On 2015-08-20 17:27:44,24
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "AbstractComponent.h"
#include "ComponentManager.h"

#include "ScriptComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

#if 0
template <size_t SIZE, size_t SORTINDEX, class ...ARGS>
struct StaticMultiArray {

	using index_t = size_t;
	using Tuple = std::tuple<ARGS...>;
	using ContainerTuple = std::tuple<std::array<ARGS, SIZE>...>;

	enum {
		Size = SIZE,
		ElementSize = sizeof(Tuple),
		SortIndex = SORTINDEX,
	};

	template<size_t index>
	using ElementType = typename std::tuple_element<index, Tuple>::type;

	template <size_t index>
	typename ElementType<index>& Get(size_t itemid) {
		auto &e = std::get<index>(m_Arrays);
		return e[itemid];
	}

	size_t GetAllocatedCount() const { return m_Allocated; }

	StaticMultiArray(): m_Allocated(0) {}
	~StaticMultiArray() {}
	StaticMultiArray(const StaticMultiArray&) = delete;
	StaticMultiArray(StaticMultiArray&&) = delete;
	StaticMultiArray& operator=(const StaticMultiArray&) = delete;

	void swap(index_t a, index_t b) { 
		//std::swap();
		//std::seque

		Swapper<0, 1, 2>(a, b);

		//throw __FUNCTION__; 
	}

	bool Alloc(ElementType<SortIndex> &Key, index_t &index) {
		index_t where;
		for (where = 0; where < m_Allocated; ++where)
			if (!(Get<SortIndex>(where) < Key))
				break;
		if (where == Size)
			return false;
		for (index_t i = m_Allocated - 1;;) {
			swap(i, i + 1);
		}
		Get<SortIndex>(where) = Key;
		index = where;
		++m_Allocated;
		return true;
	}

protected:

private:
	size_t m_Allocated;
	ContainerTuple m_Arrays;

	template<int f>
	void SwapperImpl(index_t a, index_t b) { }

	template<int f, size_t arg, size_t ...Args>
	void SwapperImpl(index_t a, index_t b) {
		auto &A = std::get<arg>(m_Arrays)[a];
		auto &B = std::get<arg>(m_Arrays)[b];
		std::swap(A, B);
		SwapperImpl<0, Args...>(a, b);
	}

	template<size_t ...Args>
	void Swapper(index_t a, index_t b) {
		SwapperImpl<0, Args...>(a, b);
	}

};

struct ComponentArray {
	friend struct ComponentArrayInfo_t;
	ComponentArray() {
	}

	void Process(const MoveConfig &config) {
		for (size_t i = 0, j = m_Array.GetAllocatedCount(); i < j; ++i) {
			m_Array.Get<0>(i)(config);
		}
	}

	bool Add(ComponentInfo& info) {
		if (!info.m_Actions.m_Process) {
			AddLog(Warning, "Attempt to register component without process function!");
			return false;
		}
		Array::index_t index;
		if (!m_Array.Alloc(info.m_Rank, index)) {
			AddLog(Error, "no more space for components!");
			return false;
		}
		m_Array.Get<0>(index) = info.m_Actions.m_Process;
		m_Array.Get<2>(index) = &info;

		return true;
	}

	size_t GetAllocatedCount() const { return m_Array.GetAllocatedCount(); }

	using Array = StaticMultiArray<StaticSettings::StaticStorage::ComponentBuffer, 1, Actions::Process, ComponentRank, ComponentInfo*>;
private:
	Array m_Array;
};

struct Impl {
	ComponentArray m_ComponentArray;

	Impl() {
	}
	~Impl() {
	}

};

static Impl *_Impl = nullptr;

#endif

ComponentManager::ComponentManager() 
	: m_UsedCount(0)
	, m_Scene(nullptr) {
}

ComponentManager::~ComponentManager() {
}

bool ComponentManager::Initialize(ciScene *scene) {
	if (!scene) {
		AddLog(Error, "No scene pointer!");
		return false;
	}
	m_Scene = scene;
	m_World = GetEngine()->GetWorld();

	if (!InstallComponent<ScriptComponent>()) {
		AddLog(Error, "Failed to install script component");
		return false;
	}

	for (size_t i = 0; i < m_UsedCount; ++i) {
		if (!m_Components[i]->Initialize()) {
			AddLogf(Error, "Failed to initialize component: %s", typeid(*m_Components[i].get()));
			return false;
		}
	}

	return true;
}

bool ComponentManager::Finalize() {
	for (size_t i = 0; i < m_UsedCount; ++i) {
		if (!m_Components[i]->Finalize()) {
			AddLogf(Error, "Failed to initialize component: %s", typeid(*m_Components[i].get()));
		}
	}
	return true;
}

bool ComponentManager::InsertComponent(UniqueAbstractComponent cptr, ComponentID cid) {
	if (m_UsedCount >= m_Components.size()) {
		AddLogf(Error, "Not enough space to install component: %s", typeid(*cptr.get()).name());
		return false;
	}

	m_Components[m_UsedCount].swap(cptr);
	m_ComponentsIDs[m_UsedCount] = cid;
	++m_UsedCount;

	return true;
}

void ComponentManager::Step(const MoveConfig &config) {
	for (size_t i = 0, j = m_UsedCount; i < j; ++i) {
		m_Components[i]->Step(config);
	}
}

AbstractComponent* ComponentManager::GetComponent(ComponentID cid) {

	//TODO: some smart search
	for (size_t i = 0; i < m_UsedCount; ++i) {
		if (m_ComponentsIDs[i] == cid) {
			return m_Components[i].get();
		}
	}

	AddLogf(Error, "There is no component with id %d", cid);
	return nullptr;
}

#if 0

//----------------------------------------------------------------

namespace Debug {

static struct ComponentArrayInfo_t : Config::Debug::MemoryInterface {
	using Array = ComponentArray::Array;
	virtual Info* GetInfo() const {
		static Info i = { 0, 0, Array::ElementSize, Array::Size, "ComponentArray" };
		i.Update(_Impl->m_ComponentArray.GetAllocatedCount());
		return &i;
	}
} ComponentArrayInf;


//static struct EntityManagerInfo_t : Debug::MemoryInterface {
//	virtual Info* GetInfo() const {
//		static Info i = { 0, sizeof(EntityIndexQueue::Item) + sizeof(EntityGenerationBuffer::Item), EntityIndexQueue::Size, "EntityManager" };
//		i.Allocated = EntityIndexQueue::Size - EMImpl.m_IndexQueue.count();
//		return &i;
//	}
//} EntityManagerInfo;

} //namespace Debug
#endif

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 
