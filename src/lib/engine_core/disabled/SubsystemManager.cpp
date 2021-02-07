#if 0
#include "SubsystemManager.h"
#include "../../Component/TransformComponent.h"
#include "ComponentRegister.h"
#include "TemplateStandardComponent.h"
#include <Core/Scripts/ScriptComponent.h>
#include <Engine/Core/Engine.h>
#include <Foundation/Component/SystemInfo.h>
#include <nfMoonGlare.h>
#include <pch.h>

namespace MoonGlare::Core::Component {

//---------------------------------------------------------------------------------------

void SubsystemManager::HandleEvent(const EntityDestructedEvent &event) {
    componentArray.RemoveAll(event.entity);
}

bool SubsystemManager::Initialize(Entity root) {
    rootEntity = root;

//#ifdef PERF_PERIODIC_PRINT
    //m_ComponentInfo.fill({});
//#endif

    for (size_t i = 0; i < m_UsedCount; ++i) {
        if (!m_Components[i]->Initialize()) {
            AddLogf(Error, "Failed to initialize component: %s", typeid(*m_Components[i].get()).name());
            return false;
        }
    }

    auto sc = GetComponent<Scripts::Component::ScriptComponent>();

    m_EventDispatcher.SetEventSink(GetWorld()->GetScriptEngine()->GetLua(), sc->GetEventSink());
    GetEventDispatcher().Register<EntityDestructedEvent>(this);

    return true;
}

//---------------------------------------------------------------------------------------

void SubsystemManager::Step(const MoveConfig &config) {
    componentArray.Step();

    //if (config.m_SecondPeriod) {
        //componentArray.DumpStatus("Periodic");
    //}

//#ifndef PERF_PERIODIC_PRINT
    //for (size_t i = 0, j = m_UsedCount; i < j; ++i) {
        //m_Components[i]->Step(config);
    //}
    //m_EventDispatcher.Step();
//#else
    auto StepStartTime = std::chrono::steady_clock::now();
    auto ComponentStartTime = StepStartTime;

    for (size_t i = 0, j = m_UsedCount; i < j; ++i) {
        m_Components[i]->Step(config);
        auto StopTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = StopTime - ComponentStartTime;
        //m_ComponentInfo[i].m_TotalStepDuration += delta.count();
        //++m_ComponentInfo[i].m_PeriodCount;
        ComponentStartTime = StopTime;
        AddData(i, delta.count());
    }

    m_EventDispatcher.Step();

    //if (config.m_SecondPeriod) {
    //    std::string oss;
    //    oss.reserve(1024);
    //    char buf[256];
    //    float sum = 0.0f;
    //    for (size_t i = 0, j = m_UsedCount; i < j; ++i) {
    //        auto &ci = m_ComponentInfo[i];
    //        float ms = (ci.m_TotalStepDuration / (float)ci.m_PeriodCount) * 1000.0f;
    //        sum += ms;
    //        ci.m_TotalStepDuration = 0.0f;
    //        ci.m_PeriodCount = 0;
    //        sprintf_s(buf, "%02x:%7.5fms | ", m_ComponentsIDs[i], ms);
    //        oss += buf;
    //    }
    //
    //    AddLogf(Performance, "SubsystemManager:%p  %sTotal:%7.5fms", this, oss.c_str(), sum);
    //}

//#endif
}

}
#endif