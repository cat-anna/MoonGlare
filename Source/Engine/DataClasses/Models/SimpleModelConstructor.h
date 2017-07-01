/*
 * cModelConstructor.h
 *
 *  Created on: 02-01-2014
 *      Author: Paweu
 */

#pragma once

namespace MoonGlare::DataClasses::Models {

class SimpleModelConstructor  {
public:
	SimpleModelConstructor();
	~SimpleModelConstructor();

	typedef int MaterialID;
	typedef int MeshID;

	struct Result {
		//std::unique_ptr<iModel> m_Model;
	};

	Renderer::MeshResourceHandle Generate(bool GenerateShape, std::unique_ptr<btBvhTriangleMeshShape> &shape) const;

//	struct EditableModelFields {
//		Graphic::VAO *VAO;
//		iSimpleModel::MeshDataVector *Meshes;
//		MaterialVector *Materials;
//		iModel *OwnerModel;
//	};
//	bool GenerateModel(EditableModelFields &request) const;

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
	public:
		Material(MaterialID id): m_id(id) {};
		MaterialID GetID() const { return m_id; };

		std::string m_TextureURI;
		std::string m_Edges;
	};

	cMesh* NewMesh();
	Material* NewMaterial();
protected:
	typedef std::vector<std::unique_ptr<cMesh>> cMeshVector;
	typedef std::vector<std::unique_ptr<Material>> MaterialVector;

	cMeshVector m_Meshes;
	MaterialVector m_Materials;
};

} // namespace MoonGlare::DataClasses::Models
