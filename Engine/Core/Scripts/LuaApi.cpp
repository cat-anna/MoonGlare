/*
 * LuaApi.cpp
 *
 *  Created on: 03-08-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include <cmath>

namespace Core {
namespace Scripts {

struct CallInfo {
	void(*func)(ApiInitializer&);
	const char* where;
	const char* Class;
};

struct ApiRegInfo {
	const GabiLib::GabiTypeInfo *Class;
	const GabiLib::GabiTypeInfo *Base;
	void(*func)(ApiInitializer&);
	const char* Where;
};

static std::list<CallInfo> *_InitFuncs = 0;
static std::list<ApiRegInfo> *_GatherList = 0;

void ApiInit::RegisterApi(void(*func)(ApiInitializer&),
							const GabiLib::GabiTypeInfo *Class,
							const GabiLib::GabiTypeInfo *BaseClass,
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

void ApiInit::Initialize(Script *s) {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	unsigned ApiInitFunctionsRun = 0;
	AddLog(Performance, "Processing api init functions");
#endif
	if (!_InitFuncs) {
		if (!_GatherList) {
			AddLog(Error, "Nothing to register!");
			return;
		}

		std::map<const GabiLib::GabiTypeInfo*, const GabiLib::GabiTypeInfo*> registered;
		_InitFuncs = new std::list < CallInfo > ;

		while (!_GatherList->empty()) {
			bool done = false;
			for (auto it = _GatherList->begin(), jt = _GatherList->end(); it != jt; ++it) {
				if (!it->Base) {
					if (it->Class)
						registered.insert(std::make_pair(it->Class, (GabiLib::GabiTypeInfo*)0));

					_InitFuncs->emplace_back(CallInfo{ it->func, it->Where, it->Class ? it->Class->Name : 0 });
					// push_back(std::make_pair(it->func, it->Where));
					_GatherList->erase(it);
					done = true;
					break;
				}

				auto regit = registered.find(it->Base);
				if (regit == registered.end())
					continue;

				registered.insert(std::make_pair(it->Class, it->Base));
				_InitFuncs->emplace_back(CallInfo{ it->func, it->Where, it->Class->Name });
				_GatherList->erase(it);
				done = true;
				break;
			}

			if (!done) {
				AddLog(Error, "Unable to register api init functions:");
				for (auto& it : *_GatherList) {
					AddLogf(Error, "%30s of base %s", it.Class->Name, it.Base->Name);
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
		if (!where)
		where = "api";
		auto api = s->GetApiInitializer().beginNamespace(where);
#if 0
		if (it.Class)
			AddLog(Debug, "Registering api of class '" << it.Class << "' in namespace " << where);
		else
			AddLog(Debug, "Registering independent api in namespace " << where);
#endif // 0
		it.func(api);
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
		++ApiInitFunctionsRun;
#endif
	}
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	AddLogf(Performance, "Executed %d api init functions.", ApiInitFunctionsRun);
#endif
}

#if 0

void cLuaApiInit::Primary(ApiDefInitializer root){
	auto Flags = root
		.beginNamespace("api")
	//	RegisterLuaFunction(&cScriptEngine::Lua_AddLog, "AddLog", root.);
	//	RegisterLuaFunction(, "rand", Lua, "integer|", m_MLua);
	/*
		.beginClass<c3DPoint>("c3DPoint")
		.addConstructor<void(*)(float, float, float)>()
		//		.Constructor()
		//		.Constructor<c3DPoint>()
		//		.Constructor<float>()
		//		.Member("x", &c3DPoint::x)
		//		.Member("y", &c3DPoint::y)
		//		.Member("z", &c3DPoint::z)
		.addFunction("Distance", &c3DPoint::Distance)
		.addFunction("DistanceXZ", &c3DPoint::DistanceXZ)
		.addFunction("Versor", &c3DPoint::Versor)
		.addFunction("VersorXZ", &c3DPoint::VersorXZ)
		.addFunction("Length", &c3DPoint::Length)
		.addFunction("LengthXZ", &c3DPoint::LengthXZ)
		.endClass()*/
	//--------------------------------AreaEvents-----------------------------------------------
	/*	.deriveClass<cBasicEventArea, cObject>("cBasicEventArea")
		.endClass()

		.deriveClass<cDamageZone, cBasicEventArea>("cDamageZone")
		.endClass()

		.deriveClass<cKillZone, cDamageZone>("cKillZone")
		.endClass()

		.deriveClass<cNearestAttackZone, cDamageZone>("cNearestAttackZone")
		.endClass()

		.deriveClass<cScriptEventArea, cBasicEventArea>("cScriptEventArea")
		.endClass()*/
	//--------------------------------Engine---------------------------------------------------------
	/*	.deriveClass<cGameEngine, cRootClass>("cGameEngine")
			.addFunction("test", &cGameEngine::test)
			.endClass()*/
	//------------------------------------------Fog---------------------------------------------------
	//	.beginClass<Graphic::cFog>("cFog")
	//		.addFunction("Enable", &cFog::Enable)
	//		.addFunction("Disable", &cFog::Disable)
	//		.addFunction("SetFog", &cFog::SetFog)
	//	.endClass()
	//------------------------------------------cMovers---------------------------------------------------
	/*.deriveClass<MoveControllers::cPathController, iMoveController>("cPathMover")
	.endClass()*/
	//--------------------------------------------------------------------------------------------
	/*ML->Insert(mluabind::Class<cGameCoords>("cGameCoords")
			.Constructor()
			.Constructor<const char*, float, float, float>()
			);*/
}
#endif

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
