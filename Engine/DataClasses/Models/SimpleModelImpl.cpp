/*
	Generated by cppsrc.sh
	On 2014-12-26 20:10:07,32
	by Paweu
*/
#include <pch.h>
#include <MoonGlare.h>
#include "SimpleModelImpl.h"

namespace DataClasses {
namespace Models {

GABI_IMPLEMENT_STATIC_CLASS(SimpleModelImpl);
ModelClassRegister::Register<SimpleModelImpl> SimpleModelImplReg("SimpleModel");

SimpleModelImpl::SimpleModelImpl(const string& Name) :
		BaseClass(Name){
}

SimpleModelImpl::~SimpleModelImpl() {
}

//------------------------------------------------------------------------

bool SimpleModelImpl::LoadFromXML(const xml_node Node) {
	if (!Node) {
		AddLog(Error, "Attempt to load model from empty node!");
		return false;
	}

	if (!BaseClass::LoadFromXML(Node))
		return false;

	string file = Node.child("File").text().as_string("");
	if (file.empty()) {
		AddLog(Error, "Model definition without file name!");
		return false;
	}

	file = (GetName() + "/") += file;
	auto fr = GetFileSystem()->OpenFile(file, DataPath::Models);
	if (!fr) {
		AddLogf(Error, "Unable to load model: %s", GetName().c_str());
		return false;
	}
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(
						fr->GetFileData(), fr->Size(),
						aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType,
						strrchr(file.c_str(), '.'));

	if (!scene) {
		AddLog(Error, "Unable to to load model file[Name:'" << GetName().c_str() << "']. Error: " << importer.GetErrorString());
		return false;
	}

	if (!DoLoadMaterials(Node, scene)) {
		AddLogf(Error, "Unable to load model materials: %s", GetName().c_str());
		return false;
	}

	if (!DoLoadMeshes(scene)) {
		AddLogf(Error, "Unable to load model meshes: %s", GetName().c_str());
		return false;
	}

	return true;
}

//------------------------------------------------------------------------

bool SimpleModelImpl::DoLoadMaterials(xml_node Node, const aiScene* scene) {
	for (unsigned i = 0; i < scene->mNumMaterials; i++) {
		const aiMaterial* pMaterial = scene->mMaterials[i];
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) <= 0)
			continue;

		m_Materials.push_back(new ModelMaterial(this, pMaterial));
	}

	return true;
}

bool SimpleModelImpl::DoLoadMeshes(const aiScene* scene) {
	Graphic::VertexVector verticles;
	Graphic::NormalVector normals;
	Graphic::TexCoordVector texCords;
	Graphic::IndexVector indices;

	unsigned NumVertices = 0, NumIndices = 0;
	m_Meshes.resize(scene->mNumMeshes);
	//	DebugLogf("[%s]: model has %d meshes", Info().c_str(), scene->mNumMeshes);
	// Count the number of vertices and indices
	for (unsigned i = 0; i < m_Meshes.size(); i++) {
		MeshData &meshd = m_Meshes[i];
		//m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		meshd.NumIndices = scene->mMeshes[i]->mNumFaces * 3;
		meshd.BaseVertex = NumVertices;
		meshd.BaseIndex = NumIndices;
		meshd.ElementMode = Graphic::Flags::fTriangles;

		NumVertices += scene->mMeshes[i]->mNumVertices;
		NumIndices += meshd.BaseIndex;
	}

	verticles.reserve(NumVertices);
	normals.reserve(NumVertices);
	texCords.reserve(NumVertices);
	indices.reserve(NumIndices);

	for (unsigned i = 0; i < m_Meshes.size(); ++i) {
		const aiMesh* mesh = scene->mMeshes[i];
		MeshData& meshd = m_Meshes[i];
		if (m_Materials.size() > mesh->mMaterialIndex)
			meshd.Material = &m_Materials[mesh->mMaterialIndex];
		else {
			if (!m_Materials.empty())
				AddLogf(Warning, "Model [%s] has mesh[id:%d] with wrong material id: %d", Info().c_str(), i, mesh->mMaterialIndex);
			meshd.Material = 0;
		}

		for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D &vertex = mesh->mVertices[j];
			aiVector3D &normal = mesh->mNormals[j];

			if (mesh->mTextureCoords[0]) {
				aiVector3D &UVW = mesh->mTextureCoords[0][j]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
				math::vec2 uv(UVW.x, UVW.y);

				texCords.push_back(uv);
			} else {
				texCords.push_back(math::vec2());
			}

			verticles.push_back(math::vec3(vertex.x, vertex.y, vertex.z));
			normals.push_back(math::vec3(normal.x, normal.y, normal.z));
		}

		for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
			aiFace *f = &mesh->mFaces[k];
			THROW_ASSERT(f->mNumIndices == 3, 0);
			for (unsigned j = 0; j < 3; ++j) {
				indices.push_back(f->mIndices[j]);
			}
		}
	}

	if (m_ShapeConstructor) {
		m_ShapeConstructor->AddTriangles(verticles, indices);
	}

	m_VAO.DelayInit(verticles, texCords, normals, indices);
/*
	m_VAO.New();
	m_VAO.AddDataChannel<3, float>(&verticles[0][0], verticles.size());
	m_VAO.AddDataChannel<2, float>(&texCords[0][0], texCords.size());
	m_VAO.AddDataChannel<3, float>(&normals[0][0], normals.size());
	m_VAO.WriteIndexes<>(&indices[0], indices.size());
	m_VAO.UnBind();
*/

	return true;
}

//------------------------------------------------------------------------

} //namespace Models 
} //namespace DataClasses 

