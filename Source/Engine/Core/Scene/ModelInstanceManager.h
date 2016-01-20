/*
 * cModelInstanceManager.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#ifndef CMODELINSTANCEMANAGER_H_
#define CMODELINSTANCEMANAGER_H_
namespace Core {
namespace Scene {

class ModelInstanceManager : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(ModelInstanceManager, cRootClass)
public:
	ModelInstanceManager(ciScene* Scene);
	~ModelInstanceManager();

	virtual bool InitalizeInstances();

	virtual void DoRender(cRenderDevice &Dev) const;
	virtual void DoRenderMeshes(cRenderDevice &Dev) const;

	virtual ModelInstancePtr CreateInstance(iModel *Model);
	virtual ModelInstancePtr CreateInstance(::DataClasses::ModelPtr Model);
	virtual ModelInstancePtr CreateInstance(const string& ModelName);

	virtual void ReleaseInstance(ModelInstance *instance);
protected:
	typedef std::list<ModelInstance*> InstanceList;

	InstanceList m_InstanceList;
	ciScene *m_Scene;

	void ReleaseAll();
};

} // namespace Scene 
} // namespace Core 
#endif // CMODELINSTANCEMANAGER_H_ 
