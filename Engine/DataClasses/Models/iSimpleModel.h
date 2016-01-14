/*
 * cSingleModel.h
 *
 *  Created on: 16-11-2013
 *      Author: Paweu
 */

#ifndef CSINGLEMODEL_H_
#define CSINGLEMODEL_H_

namespace DataClasses {
namespace Models {

class iSimpleModel : public iModel {
	GABI_DECLARE_ABSTRACT_CLASS(iSimpleModel, iModel);
public:
	iSimpleModel(const string& Name);
	virtual ~iSimpleModel();

	struct MeshData : public Graphic::VAO::MeshData {
		ModelMaterial *Material = 0;
	};
	typedef std::vector<MeshData> MeshDataVector;

	virtual void DoRender(cRenderDevice &dev) const;
	virtual void DoRenderMesh(cRenderDevice &dev) const;
	
	virtual Physics::SharedShape ConstructShape(float ShapeScale) const;
	virtual const Physics::PhysicalProperties* GetPhysicalProperties() const;

	virtual bool LoadFromXML(const xml_node Node) override;

protected:
	Physics::ShapeConstructorPtr m_ShapeConstructor;
	MaterialVector m_Materials;
	MeshDataVector m_Meshes;
	Graphic::VAO m_VAO;

	bool LoadBodyShape(xml_node node);

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
};

} // namespace Models
} // namespace DataClasses

#endif // CSINGLEMODEL_H_ 
