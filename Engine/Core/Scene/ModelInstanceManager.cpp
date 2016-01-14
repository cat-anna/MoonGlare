/*
 * cModelInstanceManager.cpp
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Scene {

GABI_IMPLEMENT_CLASS_NOCREATOR(ModelInstanceManager);

ModelInstanceManager::ModelInstanceManager(ciScene* Scene) : cRootClass(), m_Scene(Scene) {
}

ModelInstanceManager::~ModelInstanceManager() {
	ReleaseAll();
}

void ModelInstanceManager::DoRender(cRenderDevice& dev) const {
	for (auto *it : m_InstanceList) 
		if (it->GetRenderEnabled())
			it->DoRender(dev);
}

void ModelInstanceManager::DoRenderMeshes(cRenderDevice &dev) const {
	for (auto *it : m_InstanceList)
		if (it->GetRenderEnabled())
			it->DoRenderMesh(dev);
}

bool ModelInstanceManager::InitalizeInstances() {
	return true;
}

ModelInstancePtr ModelInstanceManager::CreateInstance(const string& ModelName) {
	auto m = GetDataMgr()->GetModel(ModelName);
	if (!m) {
		return ModelInstancePtr(nullptr);
	}
	return CreateInstance(m);
}

ModelInstancePtr ModelInstanceManager::CreateInstance(::DataClasses::ModelPtr Model) {
	ModelInstance *mi = new ModelInstance(Model.get(), this);
	m_InstanceList.push_back(mi);
	return ModelInstancePtr(mi);
}

ModelInstancePtr ModelInstanceManager::CreateInstance(iModel *Model) {
	ModelInstance *mi = new ModelInstance(Model, this);
	m_InstanceList.push_back(mi);
	return ModelInstancePtr(mi);
}

void ModelInstanceManager::ReleaseInstance(ModelInstance *instance) {
	m_InstanceList.remove(instance);
	delete instance;
}

void ModelInstanceManager::ReleaseAll() {
	for (auto *i : m_InstanceList) {
		delete i;
	}
	m_InstanceList.clear();
}

} // namespace Scene 
} // namespace Core 
