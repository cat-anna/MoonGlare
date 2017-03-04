/*
 * cModel.cpp
 *
 *  Created on: 12-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace MoonGlare {
namespace DataClasses {
namespace Models {

SPACERTTI_IMPLEMENT_CLASS(iModel)

iModel::iModel(const string& Name) :
	BaseClass(Name), 
	m_Materials(),
	m_Meshes(),
	m_VAO() {
}

iModel::~iModel() {
	Finalize();
}

//------------------------------------------------------------------------------------------------

void iModel::DoRender(cRenderDevice &dev) const {
	THROW_ASSERT(IsReady(), "Resource is not initialized!");
	m_VAO.Bind();
	for (auto &mesh : m_Meshes) {
		const ModelMaterial *mat = mesh.Material;
		//if (mat)
		//	dev.Bind(mat->GetMaterial());
		//else
		//	dev.BindNullMaterial();
		m_VAO.DrawElements(mesh.NumIndices, mesh.BaseIndex, mesh.BaseVertex, mesh.ElementMode);
	}
	m_VAO.UnBind();
}

void iModel::DoRenderMesh(cRenderDevice &dev) const {
	THROW_ASSERT(IsReady(), "Resource is not initialized!");
	m_VAO.Bind();
	for (auto &mesh : m_Meshes) {
		m_VAO.DrawElements(mesh);
	}
	m_VAO.UnBind();
}

//------------------------------------------------------------------------------------------------

bool iModel::DoInitialize() {
	return BaseClass::DoInitialize();
}

bool iModel::DoFinalize() {
	m_VAO.Finalize();
	m_Meshes.clear();
	m_Materials.clear();
	return BaseClass::DoFinalize();
}

} // namespace Models 
} // namespace DataClasses 
} //namespace MoonGlare 
