/*
 * cSingleModel.cpp
 *
 *  Created on: 16-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace DataClasses {
namespace Models {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(iSimpleModel)

iSimpleModel::iSimpleModel(const string& Name) :
		BaseClass(Name),
		m_Materials(),
		m_Meshes(),
		m_VAO(),
		m_ShapeConstructor()  {
}

iSimpleModel::~iSimpleModel() {
}

//----------------------------------------------------------------

bool iSimpleModel::DoInitialize() {
	return BaseClass::DoInitialize();
}

bool iSimpleModel::DoFinalize() {
	m_VAO.Finalize();
	m_Meshes.clear();
	m_Materials.clear();
	return BaseClass::DoFinalize();
}
 
//----------------------------------------------------------------

void iSimpleModel::DoRender(cRenderDevice &dev) const {
	THROW_ASSERT(IsReady(), "Resource is not initialized!");
	m_VAO.Bind();
	for (auto &mesh : m_Meshes) {
		const ModelMaterial *mat = mesh.Material; 
		if (mat)
			dev.Bind(mat->GetMaterial());
		else
			dev.BindNullMaterial();
		m_VAO.DrawElements(mesh.NumIndices, mesh.BaseIndex, mesh.BaseVertex, mesh.ElementMode);
	} 
	m_VAO.UnBind();
}

void iSimpleModel::DoRenderMesh(cRenderDevice &dev) const {
	THROW_ASSERT(IsReady(), "Resource is not initialized!");
	m_VAO.Bind();
	for (auto &mesh : m_Meshes) {
		m_VAO.DrawElements(mesh);
	} 
	m_VAO.UnBind();
}

//----------------------------------------------------------------

Physics::SharedShape iSimpleModel::ConstructShape(float ShapeScale) const {
	if (!m_ShapeConstructor) return 0;//silently ignore
	return m_ShapeConstructor->ConstructShape(ShapeScale);
}

const Physics::PhysicalProperties* iSimpleModel::GetPhysicalProperties() const {
	return &this->m_PhysicalProperties;
}

//----------------------------------------------------------------

bool iSimpleModel::LoadFromXML(const xml_node Node) {
	if (!Node) return true;

	if (!m_PhysicalProperties.LoadMeta(Node.child("Physics"))) {
		AddLog(Warning, "Unable to load model physical properties! Ignored.");
	}

	if (!LoadBodyShape(Node.child("Body"))){
		AddLog(Warning, "Unable to load model body shape!");
	}

	return true;
}

bool iSimpleModel::LoadBodyShape(xml_node node) {
	if (!node) return false;
	m_ShapeConstructor = Physics::ShapeConstructor::LoadConstructor(node);
	return m_ShapeConstructor != 0;
}

} // namespace Models
} // namespace DataClasses
