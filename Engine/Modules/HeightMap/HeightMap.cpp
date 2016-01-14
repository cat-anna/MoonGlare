#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include "HeightMap.h"

#include <Engine\DataClasses\Models\SimpleModelConstructor.h>

namespace MoonGlare {
namespace Modules {
namespace HeightMap {
	
struct HeightMapModule : public MoonGlare::Modules::ModuleInfo {
	HeightMapModule(): BaseClass("HeightMap", ModuleType::Functional) { 
		MapClassRegister::Register<HeightMap> Reg("HeightMap");
	}
};
DEFINE_MODULE(HeightMapModule);

//----------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(HeightMap);

HeightMap::HeightMap(const string& Name) :
		BaseClass(Name),
		m_MapSize(10, 10) {
}

HeightMap::~HeightMap() {
	Finalize();
}

//----------------------------------------------------------------

bool HeightMap::DoInitialize() {
	if (!BaseClass::DoInitialize()) return false;

	if (!IsLoaded() || !LoadMeta()) return false;

	if (m_MapModel) m_MapModel->Initialize();
	if (m_MapObject) m_MapObject->Initialize();
	return true;
}

bool HeightMap::DoFinalize() {
	if (m_MapModel) {
		m_MapModel.reset();
	}

	if (m_MapObject) {
		m_MapObject.reset();
	}

	return BaseClass::DoFinalize();
}

//----------------------------------------------------------------
	
bool HeightMap::LoadMeta() {
	if (IsLoaded()) return true;
	auto MapData = GetPrimaryXML();
	if (!MapData) return false;
//Options
	xml_node Options = MapData.child("Options");
	if (Options) {
		SetName(Options.child("Name").text().as_string("??"));
	}

	LoadEnvironment(MapData);

	try {
		if (!LoadStaticModel(MapData.child("HeightMap"))) throw false;
	} 
	catch (...) {
		AddLog(Error, "Unable to load static model!");
		return false;
	}

	SetLoaded(true);
	return true; 
}

::Core::Objects::StaticObject* HeightMap::LoadMapObject() {
	if (!IsLoaded() || !LoadMeta()) return 0;		
	return m_MapObject.get();
}

//----------------------------------------------------------------

bool HeightMap::LoadStaticModel(xml_node root) {
	XML::Vector::Read(root, "Size", m_MapSize, m_MapSize);

	std::unique_ptr<::DataClasses::Models::SimpleModelConstructor> constr(new ::DataClasses::Models::SimpleModelConstructor());

	using Graphic::QuadArray2;
	using Graphic::QuadArray3;

	QuadArray3 Vertex;
	QuadArray3 Normal;
	QuadArray2 TexCoords;

	for (auto &it : Normal)
		it = QuadArray3::value_type(0, 1.0f, 0);

	TexCoords[0] = QuadArray2::value_type(1, 0);
	TexCoords[1] = QuadArray2::value_type(1, 1);
	TexCoords[2] = QuadArray2::value_type(0, 1);
	TexCoords[3] = QuadArray2::value_type(0, 0);

	auto hsize = m_MapSize / 2.0f;
	math::vec2 texmult(m_MapSize[0],  m_MapSize[1]);

	for (int i = 0; i < 4; ++i)
		TexCoords[i] *= texmult;

	Vertex[0] = QuadArray3::value_type(-hsize[0], 0,  hsize[1]);
	Vertex[1] = QuadArray3::value_type(-hsize[0], 0, -hsize[1]);
	Vertex[2] = QuadArray3::value_type( hsize[0], 0, -hsize[1]);
	Vertex[3] = QuadArray3::value_type( hsize[0], 0,  hsize[1]);

	auto material = constr->NewMaterial();
	material->SetMaterialNode(root.child("Material"));
	auto mesh = constr->NewMesh();
	mesh->SelectMaterial(material->GetID());
	mesh->PushQuad(&Vertex[0], &Normal[0], &TexCoords[0]);

	constr->GenerateShape(true);
	m_MapModel.reset(constr->GenerateModel(GetName(), GetDataModule(), DataPath::Maps));

	m_MapObject = std::make_unique<::Core::Objects::StaticObject>(GetOwnerScene());
	m_MapObject->SetModelInstance(GetOwnerScene()->GetInstanceManager().CreateInstance(m_MapModel.get()));
	m_MapObject->SetName(GetName());
	m_MapObject->GetCollisionMask().Set(Physics::BodyClass::Map);
	m_MapObject->GetCollisionMask().Set(Physics::GroupMask::Map);

	return true;
}

} //namespace HeightMap 
} //namespace Modules 
} //namespace MoonGlare 
