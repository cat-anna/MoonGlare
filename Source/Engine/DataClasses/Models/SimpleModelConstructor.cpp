/*
 * cModelConstructor.cpp
 *
 *  Created on: 02-01-2014
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

#include "SimpleModelConstructor.h"

namespace MoonGlare {
namespace DataClasses {
namespace Models {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(SimpleModelConstructor)

SimpleModelConstructor::SimpleModelConstructor() {
}

SimpleModelConstructor::~SimpleModelConstructor() {
}

SimpleModelConstructor::cMesh* SimpleModelConstructor::NewMesh() {
	m_Meshes.push_back(std::make_unique<cMesh>(m_Meshes.size()));
	return m_Meshes.back().get();
}

SimpleModelConstructor::Material* SimpleModelConstructor::NewMaterial() {
	m_Materials.push_back(std::make_unique<Material>(m_Materials.size()));
	return m_Materials.back().get();
}

bool SimpleModelConstructor::Generate(bool GenerateShape, SimpleModelConstructor::Result &out) const {
	out.m_Model.reset();
	out.m_Shape.reset();

	unsigned TotalCount = 0;
	for(auto &i : m_Meshes)
		TotalCount += i->GetVerticles().size();

	if (!TotalCount) {
		AddLog(Warning, "Attempt to generate model from empty mesh!");
		return false;
	}

	auto esm = std::make_unique<iModel>("");

	Graphic::VertexVector Verticles;
	Graphic::NormalVector Normals;
	Graphic::TexCoordVector TexCoords;
	Graphic::IndexVector Index;
	TexCoords.reserve(TotalCount);
	Normals.reserve(TotalCount);
	Index.reserve(TotalCount * 3);

	auto &MeshVec = esm->GetMeshVector();
	auto &MaterialVec = esm->GetMaterialVector();

	for(auto &i : m_Meshes) {
		//int matid = -1;
		ModelMaterial *mat = 0;
		if(i->GetMaterialID() != -1){
			const Material &pmat = *m_Materials[i->GetMaterialID()];
			auto umat = std::make_unique<ModelMaterial>(esm.get(), pmat.m_Edges, pmat.m_TextureURI);
			mat = umat.get();

			MaterialVec.push_back(std::move(umat));
			//auto &m = //Model *Owner, const xml_node MaterialDef, DataPath MaterialOrigin, const string &OriginName
			//matid = esm->AddMaterial(mat.GetMaterialNode(), ModelOrigin, m_Name);
		}

		iModel::MeshData md;
		md.BaseIndex = 0;
		md.BaseVertex = Index.size();
		md.ElementMode = Graphic::Flags::fTriangles;
		md.NumIndices = i->GetVerticles().size();
		md.Material = mat;
		MeshVec.push_back(md);

		//esm->AddMesh(matid, 0/*Verticles.size()*/, Index.size(), i.GetVerticles().size());
		for(int j = 0, k = i->GetVerticles().size(); j < k; ++j) {
			Index.push_back(Verticles.size());
			Verticles.push_back(i->GetVerticles()[j]);
			TexCoords.push_back(i->GetTexCoords()[j]);
			Normals.push_back(i->GetNormals()[j]);
		}
	}

	if (GenerateShape) {
		std::unique_ptr<btTriangleMesh> mesh = std::make_unique<btTriangleMesh>();

		for (size_t i = 0; i < Index.size(); i += 3) {
			const unsigned *idx = &Index[i];

			Physics::vec3 p1 = convert(Verticles[idx[0]]);
			Physics::vec3 p2 = convert(Verticles[idx[1]]);
			Physics::vec3 p3 = convert(Verticles[idx[2]]);

			mesh->addTriangle(p1, p2, p3);
		}
		struct TriangleMeshShape : public btBvhTriangleMeshShape {
			TriangleMeshShape(btTriangleMesh *mesh, bool gen) : btBvhTriangleMeshShape(mesh, gen) { }
			~TriangleMeshShape() {
				delete m_meshInterface;
				m_meshInterface = nullptr;
			}
		};

		out.m_Shape = std::make_unique<TriangleMeshShape>(mesh.release(), true);
		//		Physics::ShapeConstructorPtr c(new Physics::TriangleMeshShapeConstructor());
//		c->AddTriangles(Verticles, Index);
//		request.ShapeConstructor->swap(c);
	}	

	esm->GetVAO().DelayInit(Verticles, TexCoords, Normals, Index);

	out.m_Model.reset(esm.release()); //TODO: this is ugly method

	return true;
}

//-------------------------------------------------------------------------------------------------

void SimpleModelConstructor::cMesh::PushTriangle(math::vec3 *vertexes, math::vec3 *Normals, math::vec2 *TexCoords) {
	if (!vertexes) return;

	m_Verticles.push_back(vertexes[0]);
	m_Verticles.push_back(vertexes[1]);
	m_Verticles.push_back(vertexes[2]);

	if (Normals) {
		m_Normals.push_back(Normals[0]);
		m_Normals.push_back(Normals[1]);
		m_Normals.push_back(Normals[2]);
	} else
		for (int i = 0; i < 3; ++i)
			m_Normals.push_back(math::vec3());

	if (TexCoords) {
		m_TexCoords.push_back(TexCoords[0]);
		m_TexCoords.push_back(TexCoords[1]);
		m_TexCoords.push_back(TexCoords[2]);
	} else
	for (int i = 0; i < 3; ++i)
		m_TexCoords.push_back(math::vec2());
}

void SimpleModelConstructor::cMesh::PushQuad(math::vec3 *vertexes, math::vec3 *Normals, math::vec2 *TexCoords) {
	if (!vertexes) return;
	PushTriangle(vertexes, Normals, TexCoords);

	m_Verticles.push_back(vertexes[0]);
	m_Verticles.push_back(vertexes[2]);
	m_Verticles.push_back(vertexes[3]);

	if (Normals) {
		m_Normals.push_back(Normals[0]);
		m_Normals.push_back(Normals[2]);
		m_Normals.push_back(Normals[3]);
	} else
		for (int i = 0; i < 3; ++i)
			m_Normals.push_back(math::vec3());

	if (TexCoords) {
		m_TexCoords.push_back(TexCoords[0]);
		m_TexCoords.push_back(TexCoords[2]);
		m_TexCoords.push_back(TexCoords[3]);
	} else
		for (int i = 0; i < 3; ++i)
			m_TexCoords.push_back(math::vec2());
}

} // namespace Models
} // namespace DataClasses
} //namespace MoonGlare 
