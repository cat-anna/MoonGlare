#pragma once

#include "../EventDispatcher.h"

namespace MoonGlare::Core::Component {

class SubsystemManager final  
		: public Config::Current::DebugMemoryInterface {
public:
	SubsystemManager();
	~SubsystemManager();

	bool Initialize(ciScene *scene, Entity root);
	bool Finalize();

	bool LoadComponents(pugi::xml_node node);

	template<class T, class ... ARGS>
	bool InstallComponent(ARGS ... args) {
		return InsertComponent(std::make_unique<T>(this, std::forward<ARGS>(args)...), T::GetComponentID());
	}

	void Step(const MoveConfig &config);

	template<class T> 
	T* GetComponent() {
		return dynamic_cast<T*>(GetComponent(T::GetComponentID()));
	}

	AbstractSystem* GetComponent(ComponentID cid);

	ciScene* GetScene() { return m_Scene; }
	World* GetWorld() { return m_World; }

	EventDispatcher& GetEventDispatcher() { return m_EventDispatcher; }
    Entity GetRootEntity() const { return rootEntity; }

	struct ComponentInfo {
		float m_TotalStepDuration;
		unsigned m_PeriodCount;
	};
private:
	std::array<UniqueAbstractSystem, Configuration::Storage::MaxComponentCount> m_Components;
	std::array<ComponentID, Configuration::Storage::MaxComponentCount> m_ComponentsIDs;
	size_t m_UsedCount;
    Entity rootEntity;
	EventDispatcher m_EventDispatcher;
	ciScene *m_Scene;
	World *m_World;

#ifdef PERF_PERIODIC_PRINT
	std::array<ComponentInfo, Configuration::Storage::MaxComponentCount> m_ComponentInfo;
#endif

	bool InsertComponent(UniqueAbstractSystem cptr, ComponentID cid);
};

} 
