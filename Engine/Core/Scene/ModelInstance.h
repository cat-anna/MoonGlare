/*
 * cModelInstance.h
 *
 *  Created on: 24-11-2013
 *      Author: Paweu
 */

#ifndef CMODELINSTANCE_H_
#define CMODELINSTANCE_H_
namespace Core {
namespace Scene {

class ModelInstance : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(ModelInstance, cRootClass)
public:
	ModelInstance(iModel *Model, ModelInstanceManager *Owner);
	virtual ~ModelInstance();

	virtual bool Initialize(Object* object);
	virtual bool Finalize();

	virtual void Release();

	virtual void DoRender(cRenderDevice &dev) const;
	virtual void DoRenderMesh(cRenderDevice &dev) const;
	virtual void Update(Object* object);

	DefineDirectSetGet(RenderEnabled, bool);
protected:
	iModel *m_Model;
	math::mat4 m_ModelMatrix;
	ModelInstanceManager *m_Owner;
	bool m_RenderEnabled = true;

	virtual void InternalInfo(std::ostringstream &buff) const;
};

} // namespace Scene 
} // namespace Core 
#endif // CMODELINSTANCE_H_ 
