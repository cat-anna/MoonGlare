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
	return true;
}

void ModelInstance::SetModel(::DataClasses::ModelPtr model) {
	m_Model.swap(model);
	if (m_Model)
		m_Model->Initialize();
}

//---------------------------------------------------------------------------------------

} // namespace Scene 
} // namespace Core 
