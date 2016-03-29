/*
 * cModelConstructor.h
 *
 *  Created on: 02-01-2014
 *      Author: Paweu
 */

#ifndef CMODELCONSTRUCTOR_H_
#define CMODELCONSTRUCTOR_H_

namespace DataClasses {
namespace Models {

class SimpleModelConstructor : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(SimpleModelConstructor, cRootClass)
public:
	SimpleModelConstructor();
	virtual ~SimpleModelConstructor();

	typedef int MaterialID;
	typedef int MeshID;

	void GenerateShape(bool Status) { m_GenerateShape = Status; }

	iSimpleModel* GenerateModel(const string& Name, DataPath ModelOrigin) const;

	struct EditableModelFields {
		Graphic::VAO *VAO;
		iSimpleModel::MeshDataVector *Meshes;
		MaterialVector *Materials;
		Physics::ShapeConstructorPtr* ShapeConstructor;
		iModel *OwnerModel;
		FileSystem::DirectoryReader Reader;
	};
	bool GenerateModel(EditableModelFields &request) const;

	class cMesh { //all primitives are triangulated
	protected:
		MeshID m_id;
		MaterialID m_MaterialID;
		Graphic::VertexVector m_Verticles;
		Graphic::NormalVector m_Normals;
		Graphic::TexCoordVector m_TexCoords;
	public:
		cMesh(MeshID id): m_id(id), m_MaterialID(-1) {};
		MeshID GetID() const { return m_id; };

		void SelectMaterial(MaterialID id) { m_MaterialID = id; }
		DefineREADAcces(MaterialID, int)

		void PushTriangle(math::vec3 *vertexes, math::vec3 *Normals, math::vec2 *TexCoords);
		void PushQuad(math::vec3 *vertexes, math::vec3 *Normals, math::vec2 *TexCoords);

		DefineREADAcces(Verticles, Graphic::VertexVector)
		DefineREADAcces(Normals, Graphic::NormalVector)
		DefineREADAcces(TexCoords, Graphic::TexCoordVector)
	};

	class Material {
	protected:
		MaterialID m_id;
		/** m_MaterialNode must be valid until mesh is generated  */
		xml_node m_MaterialNode;
	public:
		Material(MaterialID id): m_id(id) {};
		MaterialID GetID() const { return m_id; };
		DefineFULLAcces(MaterialNode, xml_node);
	};

	cMesh* NewMesh();
	Material* NewMaterial();
protected:
	typedef std::vector<std::unique_ptr<cMesh>> cMeshVector;
	typedef std::vector<std::unique_ptr<Material>> MaterialVector;
	cMeshVector m_Meshes;
	MaterialVector m_Materials;
	bool m_GenerateShape = false;
};

} // namespace Models
} // namespace DataClasses
#endif // CMODELCONSTRUCTOR_H_ 
