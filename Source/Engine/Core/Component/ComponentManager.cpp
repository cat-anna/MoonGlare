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
#include "TransformComponent.h"
#include "MeshComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

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

	Space::MemZero(m_ComponentInfo);

	if (!InstallComponent<ScriptComponent>()) {
		AddLog(Error, "Failed to install ScriptComponent");
		return false;
	}

	auto tc = std::make_unique<TransformComponent>(this);
	m_TransformComponent = tc.get();
	if (!InsertComponent(std::move(tc), TransformComponent::GetComponentID())) {
		AddLog(Error, "Failed to install TransformComponent");
		return false;
	}

	if (!InstallComponent<MeshComponent>()) {
		AddLog(Error, "Failed to install MeshComponent");
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
			sprintf_s(buf, "%3d:%7.5fms | ", m_ComponentsIDs[i], ms);
			oss += buf;
		}
		 
		AddLogf(Performance, "ComponentManager:%p  %sTotal:%7.5fms", this, oss.c_str(), sum);
	}
#endif
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

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 
