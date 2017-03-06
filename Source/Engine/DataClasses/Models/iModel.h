/*
 * cModel.h
 *
 *  Created on: 12-11-2013
 *      Author: Paweu
 */

#ifndef CMODEL_H_
#define CMODEL_H_

namespace MoonGlare {
namespace DataClasses {
namespace Models {

class iModel : public DataClass {
	SPACERTTI_DECLARE_CLASS(iModel, DataClass);
public:
	iModel(const string& Name = "");
	virtual ~iModel();

	struct MeshData : public Graphic::VAO::MeshData {
		Renderer::MaterialResourceHandle m_Material{ 0 };
	};
	typedef std::vector<MeshData> MeshDataVector;

	void DoRender(cRenderDevice &Dev) const;
	void DoRenderMesh(cRenderDevice &Dev) const;
	
//	virtual Physics::SharedShape ConstructShape(float ShapeScale) const;
//	virtual const Physics::PhysicalProperties* GetPhysicalProperties() const;
	Graphic::VAO& GetVAO() { return m_VAO; }
	MeshDataVector& GetMeshVector() { return m_Meshes; }

	virtual bool LoadFromXML(const xml_node Node) {
		return true;
	}
protected:
	MeshDataVector m_Meshes;
	Graphic::VAO m_VAO;

//	Physics::PhysicalProperties m_PhysicalProperties;

	virtual bool DoFinalize() override;
};

} // namespace Models 
} // namespace DataClasses 
} //namespace MoonGlare 

#endif // CMODEL_H_ 
