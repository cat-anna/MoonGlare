#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/DataClasses/Models/SimpleModelConstructor.h>

#include "SimpleMap.h"
#include "StaticModelLoader.h"

namespace MoonGlare {
namespace Modules {
namespace SimpleMap {

struct SimpleMapModule : public MoonGlare::Modules::ModuleInfo {
	SimpleMapModule(): BaseClass("SimpleMap", ModuleType::Functional) { 
	}

	virtual void RegisterModuleApi(ApiInitializer &api) override {
		api
			.addFunction("CreateMeshBuilder", &CreateBuilder)
			;
	}

	static StaticModelLoader CreateBuilder() {
		return StaticModelLoader();
	}
};
DEFINE_MODULE(SimpleMapModule); 

//----------------------------------------------------------------

void SimpleMapScriptApi(ApiInitializer &api) {
	api
		.beginClass<StaticModelLoader>("StaticModelLoader")
			.addProperty("UnitSize", &StaticModelLoader::GetUnitSize, &StaticModelLoader::SetUnitSize)
			.addFunction("GenerateResource", &StaticModelLoader::GenerateResource)
			.addFunction("VectorPointToCoords", &StaticModelLoader::VectorPointToCoords)

			.addFunction("AddMaterial", &StaticModelLoader::AddMaterial)
			.addFunction("AddStyle", &StaticModelLoader::AddStyle)
			.addFunction("AddJunction", &StaticModelLoader::AddJunction)
			.addFunction("AddCorridor", &StaticModelLoader::AddCorridor)

			.addFunction("SetPositionFactor", &StaticModelLoader::SetPositionFactor)
		.endClass()
		;
}
RegisterApiNonClass(SimpleMap, &SimpleMapScriptApi);

//----------------------------------------------------------------
	
#if 0

bool SimpleMap::LoadMeta() {
	if (IsLoaded()) return true;
	auto MapData = GetPrimaryXML();
	if (!MapData) return false;
//Options
	xml_node Options = MapData.child("Options");
	if (Options) {
		SetName(Options.child("Name").text().as_string("??"));
		XML::Vector::Read(Options, "UnitSize", m_UnitSize, m_UnitSize);
	}
	LoadEvents(MapData);
	LoadEnvironment(MapData);


	if (InvokeAfterLoad() != 0) 
		return false;
	SetLoaded(true);
	return true; 
}

bool SimpleMap::LoadMapObjects(Core::Objects::ObjectRegister& where) {
	auto node = GetPrimaryXML();
	if (!node) return false;
	return where.LoadObjects(node.child("Objects"), GetOwnerScene());
}

std::unique_ptr<Object> SimpleMap::LoadMapObject() {
	if (!IsLoaded() && !LoadMeta()) return 0;		
	try {
		return LoadStaticModel();
	}
	catch (...) {
		AddLog(Error, "Unable to load static model!");
	}
	return nullptr;
}

//----------------------------------------------------------------

std::unique_ptr<Object> SimpleMap::LoadStaticModel() {
	StaticModelLoader loader;
	loader.SetUnitSize(convert(m_UnitSize));
	auto reader = GetDataReader();
	FileSystem::XMLFile doc;
	if (!reader.OpenXML(doc, "StaticModel.xml")) {
		AddLog(Error, "Unable to open static model xml file! [Map: " << GetName() << "]");
		return false;
	}

	AddLog(Debug, "Generating simple map model of name " << GetName());
	if (!loader.Load(doc->document_element())) return false;
	auto exec = [this](StaticModelLoader *loader) {
		SCRIPT_INVOKE(OnModelCreate, loader, GetOwnerScene());
	};
	if (exec(&loader) != 0) return false;
	if(!loader.Validate() || !loader.Generate()) return false;

	m_MapModel.reset(loader.GetConstructor()->GenerateModel(GetName(), DataPath::Maps));

	auto MapObject = std::make_unique<Core::Objects::Object>();
	MapObject->SetOwnerScene(GetOwnerScene());
	MapObject->SetName(GetName());
	MapObject->SetModel(m_MapModel);
	MapObject->GetCollisionMask().Set(Physics::BodyClass::Map);
	MapObject->GetCollisionMask().Set(Physics::GroupMask::Map);
	MapObject->SetMass(0);
	AddLog(Debug, "Finished generating simple map model of name " << GetName());
	return std::move(MapObject);
}

#endif
} // namespace SimpleMap
} // namespace Modules
} // namespace MoonGlare
