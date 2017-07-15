/*
 * cModelConstructor.cpp
 *
 *  Created on: 02-01-2014
 *      Author: Paweu
 */
#include <pch.h>

#define NEED_MATERIAL_BUILDER
#define NEED_VAO_BUILDER
#define NEED_MESH_BUILDER

#include <MoonGlare.h>

#include "SimpleModelConstructor.h"

#include "Core/Engine.h"
#include <Renderer/Renderer.h>
#include <Renderer/Configuration/Mesh.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MeshResource.h>

namespace MoonGlare::DataClasses::Models {

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

Renderer::MeshResourceHandle SimpleModelConstructor::Generate(bool GenerateShape, std::unique_ptr<btBvhTriangleMeshShape> &shape) const {
    //out.m_Model.reset();
    shape.reset();

    unsigned TotalCount = 0;
    for(auto &i : m_Meshes)
        TotalCount += i->GetVerticles().size();

//	auto esm = std::make_unique<iModel>("");

    Graphic::VertexVector Verticles;
    Graphic::NormalVector Normals;
    Graphic::TexCoordVector TexCoords;
    Graphic::IndexVector Index;
    TexCoords.reserve(TotalCount);
    Normals.reserve(TotalCount);
    Index.reserve(TotalCount * 3);

//	auto &MeshVec = esm->GetMeshVector();

    Renderer::Configuration::Mesh::SubMeshArray meshes;
    Renderer::Configuration::Mesh::SubMeshMaterialArray materials;
    meshes.fill({});
    materials.fill({});

    std::vector<Renderer::MaterialResourceHandle> Materials;
    Materials.resize(m_Materials.size(), {});

    auto *e = Core::GetEngine();
    auto *rf = e->GetWorld()->GetRendererFacade();
    auto *resmgr = rf->GetResourceManager();

    for (size_t index = 0; index < m_Meshes.size(); ++index) {
        auto &i = m_Meshes[index];
        auto matid = i->GetMaterialID();
        if(matid != -1) {
            if (!Materials[matid].deviceHandle) {
                const auto &pmat = m_Materials[matid];

                auto matb = resmgr->GetMaterialManager().GetMaterialBuilder(Materials[matid], true);

                auto cfg = Renderer::Configuration::TextureLoad::Default();
                switch (::Space::Utils::MakeHash32(pmat->m_Edges.c_str())) {
                case "Repeat"_Hash32:
                    cfg.m_Edges = Renderer::Configuration::Texture::Edges::Repeat;
                    break;
                case "Clamp"_Hash32:
                    cfg.m_Edges = Renderer::Configuration::Texture::Edges::Clamp;
                    break;
                default:
                case "Default"_Hash32:
                    cfg.m_Edges = Renderer::Configuration::Texture::Edges::Default;
                    break;
                }
                matb.SetDiffuseMap(pmat->m_TextureURI, cfg);
                matb.SetDiffuseColor(emath::fvec4(1));
            }
            materials[index] = Materials[matid];
        }


        auto &mesh = meshes[index];
        mesh.valid = true;
        mesh.baseIndex = 0;
        mesh.baseVertex = Index.size();
        mesh.elementMode = GL_TRIANGLES;
        mesh.indexElementType = GL_UNSIGNED_INT;
        mesh.numIndices = i->GetVerticles().size();

        for(int j = 0, k = i->GetVerticles().size(); j < k; ++j) {
            Index.push_back(Verticles.size());
            Verticles.push_back(i->GetVerticles()[j]);
            TexCoords.push_back(i->GetTexCoords()[j]);
            Normals.push_back(i->GetNormals()[j]);
        }
    }

    Renderer::MeshResourceHandle mhandle;

    auto mm = &rf->GetResourceManager()->GetMeshManager();

    mm->Allocate(mhandle);

    //FIXME: ugly!
    auto smeshes = std::make_shared<decltype(meshes)>(meshes);
    auto smaterials = std::make_shared<decltype(materials)>(materials);

    //FIXME: ugly!
    rf->GetAsyncLoader()->QueueTask(std::make_shared < Renderer::FunctionalAsyncTask>(
        [mm, mhandle, Verticles, TexCoords, Normals, Index, smeshes, smaterials](Renderer::ResourceLoadStorage &storage) {
        auto tmph = mhandle;
        auto builder = mm->GetBuilder(storage.m_Queue, tmph, {}, false);
        auto &m = storage.m_Memory.m_Allocator;

        builder.subMeshArray = *smeshes;
        builder.subMeshMaterialArray = *smaterials;

        using ichannels = Renderer::Configuration::VAO::InputChannels;

        builder.AllocateVAO();
        builder.vaoBuilder.BeginDataChange();

        builder.vaoBuilder.CreateChannel(ichannels::Vertex);
        builder.vaoBuilder.SetChannelData<float, 3>(ichannels::Vertex, (const float*)m.Clone(Verticles), Verticles.size());

        builder.vaoBuilder.CreateChannel(ichannels::Texture0);
        builder.vaoBuilder.SetChannelData<float, 2>(ichannels::Texture0, (const float*)m.Clone(TexCoords), TexCoords.size());

        builder.vaoBuilder.CreateChannel(ichannels::Normals);
        builder.vaoBuilder.SetChannelData<float, 3>(ichannels::Normals, (const float*)m.Clone(Normals), Normals.size());

        builder.vaoBuilder.CreateChannel(ichannels::Index);
        builder.vaoBuilder.SetIndex(ichannels::Index, (const unsigned*)m.Clone(Index), Index.size());

        builder.vaoBuilder.EndDataChange();
        builder.vaoBuilder.UnBindVAO();
        builder.Commit();
    }));

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

        shape = std::make_unique<TriangleMeshShape>(mesh.release(), true);
//		Physics::ShapeConstructorPtr c(new Physics::TriangleMeshShapeConstructor());
//		c->AddTriangles(Verticles, Index);
//		request.ShapeConstructor->swap(c);
    }	

    return mhandle;
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

} // namespace MoonGlare::DataClasses::Models
