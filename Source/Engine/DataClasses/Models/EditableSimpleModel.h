/*
 * cEditableSimpleModel.h
 *
 *  Created on: 03-01-2014
 *      Author: Paweu
 */
#ifndef CEDITABLESIMPLEMODEL_H_
#define CEDITABLESIMPLEMODEL_H_

namespace MoonGlare {
namespace DataClasses {
namespace Models {

class EditableSimpleModel : public iSimpleModel {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(EditableSimpleModel, iSimpleModel)
public:
	EditableSimpleModel(const string& Name);
	virtual ~EditableSimpleModel();

	Graphic::VAO& GetVAO() { return m_VAO; }
	MeshDataVector& GetMeshVector() { return m_Meshes; }
	MaterialVector& GetMaterialVector() { return m_Materials; }
	Physics::ShapeConstructorPtr& GetShapeConstructor() { return m_ShapeConstructor; }

protected:
	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
};

} // namespace Models
} // namespace DataClasses
} //namespace MoonGlare 

#endif // CEDITABLESIMPLEMODEL_H_ 
