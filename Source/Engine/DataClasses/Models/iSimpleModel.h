/*
 * cSingleModel.h
 *
 *  Created on: 16-11-2013
 *      Author: Paweu
 */

#ifndef CSINGLEMODEL_H_
#define CSINGLEMODEL_H_

namespace MoonGlare {
namespace DataClasses {
namespace Models {

class iSimpleModel : public iModel {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iSimpleModel, iModel);
public:
	iSimpleModel(const string& Name);
	virtual ~iSimpleModel();

	struct MeshData : public Graphic::VAO::MeshData {
		ModelMaterial *Material = 0;
	};
	typedef std::vector<MeshData> MeshDataVector;

	virtual void DoRender(cRenderDevice &dev) const;
	virtual void DoRenderMesh(cRenderDevice &dev) const;
	
	virtual bool LoadFromXML(const xml_node Node) override;

protected:
	MaterialVector m_Materials;
	MeshDataVector m_Meshes;
	Graphic::VAO m_VAO;

	bool LoadBodyShape(xml_node node);

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
};

} // namespace Models
} // namespace DataClasses
} //namespace MoonGlare 

#endif // CSINGLEMODEL_H_ 
