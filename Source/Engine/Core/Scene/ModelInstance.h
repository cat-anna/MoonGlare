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

class ModelInstance {
public:
	ModelInstance();
	~ModelInstance();

	bool GetPhysicalSettings(Object* object);

	void SetModel(::DataClasses::ModelPtr model);
	::DataClasses::ModelPtr GetModel() const { return m_Model; }
protected:
	::DataClasses::ModelPtr m_Model;
	math::mat4 m_ModelMatrix;
};

} // namespace Scene 
} // namespace Core 
#endif // CMODELINSTANCE_H_ 
