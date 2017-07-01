#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/DataClasses/Models/SimpleModelConstructor.h>

#include "SimpleMap.h"
#include "StaticModelLoader.h"

namespace MoonGlare {
namespace Modules {
namespace SimpleMap {

struct SimpleMapModule : public MoonGlare::Modules::ModuleInfo {
	SimpleMapModule(): BaseClass("SimpleMapBuilder", ModuleType::Functional) { 
	}

	virtual void RegisterModuleApi(ApiInitializer &api) override {
		api
			.addFunction("CreateMeshBuilder", &CreateBuilder)
			;
	}

	virtual void RegisterInternalApi(ApiInitializer &api) override {
	api
		.beginClass<StaticModelLoader>("StaticModelLoader")
			.addProperty("UnitSize", &StaticModelLoader::GetUnitSize, &StaticModelLoader::SetUnitSize)
			.addProperty("DoubleWalls", &StaticModelLoader::GetDoubleWalls, &StaticModelLoader::SetDoubleWalls)

			.addFunction("Generate", &StaticModelLoader::Generate)
			.addFunction("GetBodyShape", &StaticModelLoader::GetBodyShape)
		
			.addFunction("VectorPointToCoords", &StaticModelLoader::VectorPointToCoords)

			.addFunction("AddMaterial", &StaticModelLoader::AddMaterial)
			.addFunction("AddStyle", &StaticModelLoader::AddStyle)
			.addFunction("AddJunction", &StaticModelLoader::AddJunction)
			.addFunction("AddCorridor", &StaticModelLoader::AddCorridor)

			.addFunction("SetPositionFactor", &StaticModelLoader::SetPositionFactor)
		.endClass()
		;
	}

	static StaticModelLoader CreateBuilder() {
		return StaticModelLoader();
	}
};
DEFINE_MODULE(SimpleMapModule); 

//----------------------------------------------------------------

} // namespace SimpleMap
} // namespace Modules
} // namespace MoonGlare
