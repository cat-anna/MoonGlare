#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/Objects/StaticObject.h>
#include <Engine/DataClasses/Models/SimpleModelConstructor.h>

#include "SimpleMap.h"
#include "StaticModelLoader.h"

namespace MoonGlare {
namespace Modules {
namespace SimpleMap {

struct SimpleMapModule : public MoonGlare::Modules::ModuleInfo {
	SimpleMapModule(): BaseClass("SimpleMap", ModuleType::Functional) { 
		MapClassRegister::Register<SimpleMap> Reg("SimpleMap");
	}
};
DEFINE_MODULE(SimpleMapModule); 

//----------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(SimpleMap);
IMPLEMENT_SCRIPT_EVENT_VECTOR(SimpleMapScriptEvents);
RegisterApiDerivedClass(SimpleMap, &SimpleMap::RegisterScriptApi);

SimpleMap::SimpleMap(const string& Name) :
		BaseClass(Name),
		m_MapObject(),
		m_UnitSize(1, 1, 1) {
	SetScriptHandlers(new SimpleMapScriptEvents());
}

SimpleMap::~SimpleMap() {
}

//----------------------------------------------------------------

void SimpleMap::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cSimpleMap")
		//.addFunction("GetTexture", &DataModule::GetTexture_char)
		//.addFunction("GetPatternObject", &DataModule::GetPatternObject_char)
	.endClass()
		.beginClass<StaticModelLoader>("StaticModelLoader") 
		.addFunction("AddJunction", &StaticModelLoader::AddJunction)
		.addFunction("AddCorridor", &StaticModelLoader::AddCorridor)
		.addFunction("AddStyle", &StaticModelLoader::AddStyle)
		.addFunction("VectorPointToCoords", &StaticModelLoader::VectorPointToCoords)
		.addFunction("SetPositionFactor", &StaticModelLoader::SetPositionFactor)
	.endClass()
	;
}

//----------------------------------------------------------------

bool SimpleMap::DoInitialize() {
	if (!BaseClass::DoInitialize()) return false;

	if (!IsLoaded() && !LoadMeta()) return false;

	if (m_MapModel) m_MapModel->Initialize();
	if (m_MapObject) m_MapObject->Initialize();

	return true;
}

bool SimpleMap::DoFinalize() {
	if (m_MapModel) {
		m_MapModel->Finalize();
		m_MapModel.reset(0);
	}

	if (m_MapObject) {
		m_MapObject->Finalize();
		m_MapObject.reset(0);
	}

	return BaseClass::DoFinalize();
}

//----------------------------------------------------------------
	
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

	try {
		if (!LoadStaticModel()) throw false;
	} 
	catch (...) {
		AddLog(Error, "Unable to load static model!");
		return false;
	}

	if (InvokeAfterLoad() != 0) 
		return false;
	SetLoaded(true);
	return true; 
}

bool SimpleMap::LoadMapObjects(::Core::Objects::ObjectRegister& where) {
	auto node = GetPrimaryXML();
	if (!node) return false;
	return where.LoadObjects(node.child("Objects"), GetOwnerScene());
}

::Core::Objects::StaticObject* SimpleMap::LoadMapObject() {
	if (!IsLoaded() && !LoadMeta()) return 0;		
	return m_MapObject.get();
}

//----------------------------------------------------------------

bool SimpleMap::LoadStaticModel() {
	StaticModelLoader loader(convert(m_UnitSize));

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

	m_MapModel.reset(loader.GetConstructor()->GenerateModel(GetName(), GetDataModule(), DataPath::Maps));

	m_MapObject = std::make_unique<::Core::Objects::StaticObject>(GetOwnerScene());
	m_MapObject->SetModelInstance(GetOwnerScene()->GetInstanceManager().CreateInstance(m_MapModel.get()));
	m_MapObject->SetName(GetName());
	m_MapObject->GetCollisionMask().Set(Physics::BodyClass::Map);
	m_MapObject->GetCollisionMask().Set(Physics::GroupMask::Map);
	AddLog(Debug, "Finished generating simple map model of name " << GetName());
	return true;
}

//----------------------------------------------------------------

int SimpleMap::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int SimpleMap::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }
int SimpleMap::InvokeAfterLoad() { SCRIPT_INVOKE(AfterLoad, GetOwnerScene()); }

} // namespace SimpleMap
} // namespace Modules
} // namespace MoonGlare
