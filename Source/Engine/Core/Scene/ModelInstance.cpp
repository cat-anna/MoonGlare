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

GABI_IMPLEMENT_CLASS_NOCREATOR(ModelInstance)

ModelInstance::ModelInstance(iModel *Model, ModelInstanceManager* Owner) :
		BaseClass(),
		m_Model(Model),
		m_ModelMatrix(),
		m_Owner(Owner) {
}

ModelInstance::~ModelInstance() {
}

//---------------------------------------------------------------------------------------

void ModelInstanceDeleter::operator()(ModelInstance* i) {
	i->Release();
}

//---------------------------------------------------------------------------------------

void ModelInstance::InternalInfo(std::ostringstream& buff) const {
	BaseClass::InternalInfo(buff);
	//buff << " Model:" << m_Model->GetName() << " instc:" << InstancesCount();
}

//---------------------------------------------------------------------------------------

bool ModelInstance::Initialize(Object* object) {
	m_Model->Initialize();
	object->SetShape(m_Model->ConstructShape(object->GetScale()));
	object->SetPhysicalProperties(m_Model->GetPhysicalProperties());
	Update(object);
	return true;
}

bool ModelInstance::Finalize() {
	LOG_NOT_IMPLEMENTED();
	return true;
}

void ModelInstance::Update(Object* object) {
	//math::mat4 mat = glm::translate(glm::mat4(), state.Position);
	//m_ModelMatrix = glm::rotate(m_ModelMatrix, m_Rotation[1], glm::vec3(0, -1, 0));
	//m_ModelMatrix = glm::scale(mat, state.Scale);
	object->GetMotionState().GetGLMatrix(m_ModelMatrix);
	//m_ModelMatrix = glm::scale(m_ModelMatrix, math::vec3(state.Scale));
	float scale = object->GetScale();
	m_ModelMatrix[0] *= scale;
	m_ModelMatrix[1] *= scale;
	m_ModelMatrix[2] *= scale;
}

void ModelInstance::Release() {
	m_Owner->ReleaseInstance(this);
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
