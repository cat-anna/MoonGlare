#include <pch.h>
#include <MoonGlare.h>
#include <cmath>

#include <Core/Scripts/LuaApi.h>
#include <Core/Scripts/ScriptEngine.h>

#include <Modules/ModuleManager.h>
#include <Core/Component/ComponentRegister.h>

#include <EngineBase/Component/EventInfo.h>
#include <EngineBase/Component/ComponentInfo.h>

namespace MoonGlare {
namespace Core {
namespace Scripts {

struct CallInfo {
    void(*func)(ApiInitializer&);
    const char* where;
    const char* Class;
};

struct ApiRegInfo {
    const Space::RTTI::TypeInfo *Class;
    const Space::RTTI::TypeInfo *Base;
    void(*func)(ApiInitializer&);
    const char* Where;
};

static std::list<CallInfo> *_InitFuncs = 0;
static std::list<ApiRegInfo> *_GatherList = 0;

void ApiInit::RegisterApi(void(*func)(ApiInitializer&),
                            const Space::RTTI::TypeInfo *Class,
                            const Space::RTTI::TypeInfo *BaseClass,
                            const char *where) {
    ApiRegInfo reg;
    reg.Base = BaseClass;
    reg.Class = Class;
    reg.func = func;
    reg.Where = where;
    if (!_GatherList)
        _GatherList = new std::list<ApiRegInfo>();
    _GatherList->push_back(reg);
}

void ApiInit::Initialize(ScriptEngine *s) {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
    std::chrono::high_resolution_clock::time_point tstart = std::chrono::high_resolution_clock::now();
#endif

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
    unsigned ApiInitFunctionsRun = 0;
    AddLog(Performance, "Processing api init functions");
#endif
    if (!_InitFuncs) {
        if (!_GatherList) {
            AddLog(Error, "Nothing to register!");
            return;
        }

        std::map<const Space::RTTI::TypeInfo*, const Space::RTTI::TypeInfo*> registered;
        _InitFuncs = new std::list < CallInfo > ;

        while (!_GatherList->empty()) {
            bool done = false;
            for (auto it = _GatherList->begin(), jt = _GatherList->end(); it != jt; ++it) {
                if (!it->Base) {
                    if (it->Class)
                        registered.insert(std::make_pair(it->Class, (Space::RTTI::TypeInfo*)0));

                    _InitFuncs->emplace_back(CallInfo{ it->func, it->Where, it->Class ? it->Class->GetName() : 0 });
                    // push_back(std::make_pair(it->func, it->Where));
                    _GatherList->erase(it);
                    done = true;
                    break;
                }

                auto regit = registered.find(it->Base);
                if (regit == registered.end())
                    continue;

                registered.insert(std::make_pair(it->Class, it->Base));
                _InitFuncs->emplace_back(CallInfo{ it->func, it->Where, it->Class->GetName() });
                _GatherList->erase(it);
                done = true;
                break;
            }

            if (!done) {
                AddLog(Error, "Unable to register api init functions:");
                for (auto& it : *_GatherList) {
                    AddLogf(Error, "%30s of base %s", it.Class->GetName(), it.Base->GetName());
                }
                AddLog(Error, "Reason: broken inheritance tree");
                break;
            }
        }

        _GatherList->clear();
        delete _GatherList;
        _GatherList = 0;
    }

    for (auto &it : *_InitFuncs) {
        const char *where = it.where;
#if 0
        if (it.Class)
            AddLog(Debug, "Registering api of class '" << it.Class << "' in namespace " << where);
        else
            AddLog(Debug, "Registering independent api in namespace " << where);
#endif // 0
        if (where) {
            s->GetApiInitializer()
                .beginNamespace(where)
                    .DefferCalls([it](auto &n) { it.func(n); });
        } else {
            s->GetApiInitializer() 
                .DefferCalls([it](auto &n) { it.func(n); });
        }
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
        ++ApiInitFunctionsRun;
#endif
    }

    for(auto *it : *MoonGlare::GetModulesManager()->GetModuleList()) {
        using MoonGlare::Modules::ModuleInfo;
        s->GetApiInitializer()
            .beginNamespace("Module")
                .beginNamespace(it->GetName())
                    .DefferCalls<ModuleInfo, &ModuleInfo::RegisterModuleApi>(it);

        s->GetApiInitializer()
            .beginNamespace("api")
                .beginNamespace(it->GetName())
                    .DefferCalls<ModuleInfo, &ModuleInfo::RegisterInternalApi>(it);

        ApiInitFunctionsRun += 2;
    }
    {
        {
            auto nComponent = s->GetApiInitializer().beginNamespace("Component");
            for (auto &it : Component::ComponentRegister::GetComponentMap()) {
                auto &ci = *it.second;
                if (!ci.m_Flags.m_RegisterID)
                    continue;
                nComponent.addProperty(ci.m_Name, ci.m_GetCID, (void(*)(int))nullptr);
            }
            using BaseComponentInfo = MoonGlare::Component::BaseComponentInfo;
            auto maxid = BaseComponentInfo::GetUsedComponentTypes();
            for (decltype(maxid) it = 0; it < maxid; ++it) {
                auto &info = BaseComponentInfo::GetComponentTypeInfo(it);
                if (info.apiInitFunc) {
                    nComponent.addVariable(info.componentName, const_cast<Component::ComponentClassId*>(&info.id), false);
                }
            }
            nComponent.endNamespace();
            ++ApiInitFunctionsRun;
        }
        for (auto &it : Component::ComponentRegister::GetComponentMap()) {
            auto &ci = *it.second;
            if (!ci.m_ApiRegFunc)
                continue;
            ++ApiInitFunctionsRun;
            s->GetApiInitializer()
                .beginNamespace("api")
                    .beginNamespace("Component")
                        .DefferCalls([ci](auto &n) { ci.m_ApiRegFunc(n); });
        }
    }

    {
        using BaseEventInfo = MoonGlare::Component::BaseEventInfo;
        auto maxid = BaseEventInfo::GetUsedEventTypes();
        for (decltype(maxid) it = 0; it < maxid; ++it) {
            auto info = BaseEventInfo::GetEventTypeInfo(it);
            if (info.apiInitFunc) {
                ++ApiInitFunctionsRun;
                s->GetApiInitializer()
                    .beginNamespace("api")
                        .beginNamespace("Event")
                            .DefferCalls([&info](auto &n) { info.apiInitFunc(n); });
            }
        }
    }
    {
        using BaseComponentInfo = MoonGlare::Component::BaseComponentInfo;
        auto maxid = BaseComponentInfo::GetUsedComponentTypes();
        for (decltype(maxid) it = 0; it < maxid; ++it) {
            auto info = BaseComponentInfo::GetComponentTypeInfo(it);
            if (info.apiInitFunc) {
                ++ApiInitFunctionsRun;
                s->GetApiInitializer()
                    .beginNamespace("api")
                        .beginNamespace("Component")
                            .DefferCalls([&info](auto &n) { info.apiInitFunc(n); });
            }
        }
    }
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
    std::chrono::high_resolution_clock::time_point tend = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tend - tstart).count() / 1000.0f;
    AddLogf(Performance, "Executed %d api init functions. Took %.3f ms", ApiInitFunctionsRun, duration );
#endif
}

//-------------------------------------------------

} //namespace Scritps
} //namespace Core
} //namespace MoonGlare
