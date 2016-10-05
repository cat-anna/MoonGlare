/*
 * LuaApi.cpp
 *
 *  Created on: 03-08-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include <cmath>

#include <ModulesManager.h>
#include <Core/Component/ComponentRegister.h>

namespace luabridge {

LuaBridgeApiDump *gLuaBridgeApiDump = nullptr;
static LuaBridgeApiDump gLuaBridgeApiDumpInstance;

template<typename ... ARGS>
static void ApiLine(const char *fmt, ARGS&& ... args) {
	char buf[4096];
	sprintf_s(buf, fmt, std::forward<ARGS>(args)...);
	gLuaBridgeApiDumpInstance.m_output << buf << "\n";// << std::flush;
}

static void ResetLocation() { ApiLine("ResetLocation()"); }

void LuaBridgeApiDump::beginNamespace(const char *name) { ApiLine("BeginNamespace([[%s]])", name); }
void LuaBridgeApiDump::endNamespace() { ApiLine("EndNamespace()"); }
void LuaBridgeApiDump::beginClass(const char *name, const char *cname) { ApiLine("BeginClass([[%s]], [[%s]])", name, cname); }
void LuaBridgeApiDump::deriveClass(const char *name, const char *cname, const char *bname) { ApiLine("DeriveClass([[%s]], [[%s]], { [[%s]], })", name, cname, bname); }
void LuaBridgeApiDump::endClass() { ApiLine("EndClass()"); }

}

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

	luabridge::gLuaBridgeApiDump = &luabridge::gLuaBridgeApiDumpInstance;
	luabridge::gLuaBridgeApiDumpInstance.m_output.open("logs/luaapi.lua", std::ios::out);
	
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
					.DefferCalls([it](auto &n) {it.func(n); });
		} else {
			s->GetApiInitializer()
				.DefferCalls([it](auto &n) {it.func(n); });
		}
		luabridge::ResetLocation();
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
		luabridge::ResetLocation();

		s->GetApiInitializer()
			.beginNamespace("api")
				.beginNamespace(it->GetName())
					.DefferCalls<ModuleInfo, &ModuleInfo::RegisterInternalApi>(it);
		luabridge::ResetLocation();

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
			nComponent.endNamespace();
			++ApiInitFunctionsRun;
		}
		for (auto &it : Component::ComponentRegister::GetComponentMap()) {
			auto &ci = *it.second;
			if (!ci.m_ApiRegFunc)
				continue;

			s->GetApiInitializer()
				.beginNamespace("api")
					.beginNamespace("Component")
						.DefferCalls([ci](auto &n) { ci.m_ApiRegFunc(n); });
			luabridge::ResetLocation();

			++ApiInitFunctionsRun;
		}
	}

	luabridge::gLuaBridgeApiDumpInstance.m_output.flush();
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	std::chrono::high_resolution_clock::time_point tend = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tend - tstart).count() / 1000.0f;
	AddLogf(Performance, "Executed %d api init functions. Took %.3f ms", ApiInitFunctionsRun, duration );
#endif
}

struct consttable_t {
	const char *name;
	int value;
} consttable[] = {
	//{ "Destructible", SPRITE_FLAG_DESTROYABLE },
	//{ "Collision", SPRITE_FLAG_COLLISION },
	//{ "Dead", SPRITE_FLAG_DEAD },
	//{ "Visible", SPRITE_FLAG_VISIBLE },
	//{ "Error", (int)Log::LogLineType::Error },
	//{ "Warning", (int)Log::LogLineType::Warning },
	//{ "Hint", (int)Log::LogLineType::Hint },
	//	{ "", },
	//	{ "", },
	{ 0, 0 },
};

void FlagsScriptApi(ApiInitializer &root) {
	//for (consttable_t *i = consttable; i->name; ++i)
		//root.addProperty(i->name, &i->value, false);
}
//RegisterApiNonClass(ScriptApi, &ScriptApi, "Flags");

//void ScriptApi(ApiInitializer &root){
	//root;
//}
//RegisterApiNonClass(ScriptApi, &ScriptApi, "math");

//-------------------------------------------------

} //namespace Scritps
} //namespace Core
} //namespace MoonGlare
