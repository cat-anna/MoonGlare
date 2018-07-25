#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/Engine.h>
#include "SubsystemManager.h"
#include "ComponentRegister.h"

#include "AbstractSystem.h"
#include "TransformComponent.h"

#include <Core/Scripts/ScriptComponent.h>

namespace MoonGlare::Core::Component {

SubsystemManager::SubsystemManager() 
	: m_UsedCount(0)
	, m_Scene(nullptr) {

	m_World = GetEngine()->GetWorld();//TODO

	DebugMemorySetClassName("SubsystemManager");
	DebugMemoryRegisterCounter("IndexUsage", [this](DebugMemoryCounter& counter) {
		counter.Allocated = m_UsedCount;
		counter.Capacity = m_ComponentsIDs.size();
		counter.ElementSize = sizeof(UniqueAbstractSystem) + sizeof(ComponentID);
	});
}

SubsystemManager::~SubsystemManager() {
}

//---------------------------------------------------------------------------------------

bool SubsystemManager::Initialize(ciScene *scene, Entity root) {
	if (!scene) {
		AddLog(Error, "No scene pointer!");
		return false;
	}
	m_Scene = scene;
    rootEntity = root;

					  
#ifdef PERF_PERIODIC_PRINT
	Space::MemZero(m_ComponentInfo);
#endif
	
	for (size_t i = 0; i < m_UsedCount; ++i) {
		if (!m_Components[i]->Initialize()) {
			AddLogf(Error, "Failed to initialize component: %s", typeid(*m_Components[i].get()).name());
			return false;
		}
	}

    auto sc = GetComponent<Scripts::Component::ScriptComponent>();

    m_EventDispatcher.Initialize(GetWorld(), sc->GetEventSink());

	return true;
}

bool SubsystemManager::Finalize() {
    m_EventDispatcher.Finalize();

	for (size_t i = 0; i < m_UsedCount; ++i) {
		if (!m_Components[i]->Finalize()) {
			AddLogf(Error, "Failed to initialize component: %s", typeid(*m_Components[i].get()).name());
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------

bool SubsystemManager::LoadComponents(pugi::xml_node node) {
#ifdef PERF_PERIODIC_PRINT
	Space::MemZero(m_ComponentInfo);
#endif
	m_UsedCount = 0;
	if (!node) {
		AddLog(Warning, "Attempt to load components from invalid node!");
		return true;
	}

	for (auto it = node.child("Component"); it; it = it.next_sibling("Component")) {
		ComponentID cid = ComponentID::Invalid;

		if (!Component::ComponentRegister::ExtractCIDFromXML(it, cid)) {
			AddLogf(Warning, "Unknown component!");
			continue;
		}

		auto *info = ComponentRegister::GetComponentInfo(cid);
		if (!info) {
			AddLogf(Error, "Unknown CID: %d", cid);
			continue;
		}

		auto cptr = info->m_CreateFunc(this);
		if (!cptr) {
			AddLogf(Error, "Failed to create CID: %d", cid);
			continue;
		}

		if (!cptr->LoadComponentConfiguration(it)) {
			AddLogf(Error, "Failed to load component configuration CID: %d", cid);;
			continue;
		}

		AddLogf(Hint, "Installing component cid:%d (%s)", cid, typeid(*cptr.get()).name());
		if (!InsertComponent(std::move(cptr), cid)) {
			AddLog(Error, "Failed to install TransformComponent");
			continue;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------

bool SubsystemManager::InsertComponent(UniqueAbstractSystem cptr, ComponentID cid) {
	if (m_UsedCount >= m_Components.size()) {
		AddLogf(Error, "Not enough space to install component: %s", typeid(*cptr.get()).name());
		return false;
	}

	m_Components[m_UsedCount].swap(cptr);
	m_ComponentsIDs[m_UsedCount] = cid;
	++m_UsedCount;

	return true;
}

void SubsystemManager::Step(const MoveConfig &config) {
	if (m_UsedCount == 0) {
		// nothing to do
		return;
	}
#ifdef PERF_PERIODIC_PRINT
	auto StepStartTime = std::chrono::steady_clock::now();
	auto ComponentStartTime = StepStartTime;
#endif

	for (size_t i = 0, j = m_UsedCount; i < j; ++i) {
		m_Components[i]->Step(config);
#ifdef PERF_PERIODIC_PRINT
		auto StopTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> delta = StopTime - ComponentStartTime;
		m_ComponentInfo[i].m_TotalStepDuration += delta.count();
		++m_ComponentInfo[i].m_PeriodCount;
		ComponentStartTime = StopTime;
#endif
	}
#ifdef PERF_PERIODIC_PRINT
	if (config.m_SecondPeriod) {
		std::string oss;
		oss.reserve(1024);
		char buf[256];
		float sum = 0.0f;
		for (size_t i = 0, j = m_UsedCount; i < j; ++i) {
			auto &ci = m_ComponentInfo[i];
			float ms = (ci.m_TotalStepDuration / (float)ci.m_PeriodCount) * 1000.0f;
			sum += ms;
			ci.m_TotalStepDuration = 0.0f;
			ci.m_PeriodCount = 0;
			sprintf_s(buf, "%02x:%7.5fms | ", m_ComponentsIDs[i], ms);
			oss += buf;
		}
		 
		AddLogf(Performance, "SubsystemManager:%p  %sTotal:%7.5fms", this, oss.c_str(), sum);
	}
#endif
}

AbstractSystem* SubsystemManager::GetComponent(ComponentID cid) {

	//TODO: some smart search
	for (size_t i = 0; i < m_UsedCount; ++i) {
		if (m_ComponentsIDs[i] == cid) {
			return m_Components[i].get();
		}
	}

	AddLogf(Error, "There is no component with id %d", cid);
	return nullptr;
}

}
