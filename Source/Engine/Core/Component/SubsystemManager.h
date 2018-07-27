#pragma once

#include "../EventDispatcher.h"

#include <EngineBase/Component/iSubsystem.h>

namespace MoonGlare::Core::Component {

using namespace MoonGlare::Component;

class SubsystemManager final  
        : public MoonGlare::Component::iSubsystemManager
		, public Config::Current::DebugMemoryInterface {
public:
	SubsystemManager();
	~SubsystemManager() override;

	bool Initialize(ciScene *scene, Entity root);
	bool Finalize();

	bool LoadComponents(pugi::xml_node node);

    InterfaceMap& GetInterfaceMap() override { return *m_World; }
    ComponentArray& GetComponentArray() override { return *componentArray; }

	template<class T, class ... ARGS>
	bool InstallComponent(ARGS ... args) {
		return InsertComponent(std::make_unique<T>(this, std::forward<ARGS>(args)...), T::GetComponentID());
	}

	void Step(const MoveConfig &config);

	template<class T> 
	T* GetComponent() {
		return dynamic_cast<T*>(GetComponent(T::GetComponentID()));
	}

    iSubsystem* GetComponent(ComponentID cid);

	ciScene* GetScene() { return m_Scene; }
	World* GetWorld() { return m_World; }

	EventDispatcher& GetEventDispatcher() { return m_EventDispatcher; }
    Entity GetRootEntity() const { return rootEntity; }

	struct ComponentInfo {
		float m_TotalStepDuration;
		unsigned m_PeriodCount;
	};
private:
    using Storage = MoonGlare::Configuration::Storage;
        
	std::array<UniqueSubsystem, Storage::MaxComponentCount> m_Components;
	std::array<ComponentID, Storage::MaxComponentCount> m_ComponentsIDs;
    size_t m_UsedCount;
    Entity rootEntity;
	EventDispatcher m_EventDispatcher;
	ciScene *m_Scene;
	World *m_World;

    std::unique_ptr<MoonGlare::Component::ComponentArray> componentArray;

#ifdef PERF_PERIODIC_PRINT
	std::array<ComponentInfo, Storage::MaxComponentCount> m_ComponentInfo;
#endif

	bool InsertComponent(UniqueSubsystem cptr, ComponentID cid);
};

} 
