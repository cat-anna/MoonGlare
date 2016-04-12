/*
 * cModelInstance.cpp
 *
 *  Created on: 24-11-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Scene {

ModelInstance::ModelInstance() :
		m_Model(nullptr),
		m_ModelMatrix() {
}

ModelInstance::~ModelInstance() {
}

//---------------------------------------------------------------------------------------

bool ModelInstance::GetPhysicalSettings(Object* object) {
	if (!m_Model)
		return false;
	object->SetShape(m_Model->ConstructShape(object->GetScale()));
	object->SetPhysicalProperties(m_Model->GetPhysicalProperties());
	Update(object);
	return true;
}

void ModelInstance::SetModel(::DataClasses::ModelPtr model) {
	m_Model.swap(model);
	if (m_Model)
		m_Model->Initialize();
}

void ModelInstance::Update(Object* object) {
	if (!m_Model)
		return;
	object->GetPositionTransform().getOpenGLMatrix((float*)&m_ModelMatrix);
	float scale = object->GetScale();
	m_ModelMatrix[0] *= scale;
	m_ModelMatrix[1] *= scale;
	m_ModelMatrix[2] *= scale;
}

//---------------------------------------------------------------------------------------

void ModelInstance::DoRender(cRenderDevice& dev) const {
	dev.SetModelMatrix(m_ModelMatrix);
	m_Model->DoRender(dev);
}

void ModelInstance::DoRenderMesh(cRenderDevice& dev) const {
	dev.SetModelMatrix(m_ModelMatrix);
	m_Model->DoRenderMesh(dev);
}

} // namespace Scene 
} // namespace Core 
