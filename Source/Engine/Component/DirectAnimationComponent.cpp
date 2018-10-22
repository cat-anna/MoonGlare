#include <pch.h>
#include <nfMoonGlare.h>

#include "DirectAnimationComponent.h"

#include <Core/Component/ComponentRegister.h>
#include <Component/TransformComponent.h>

#include <DirectAnimationComponent.x2c.h>

#define NEED_VAO_BUILDER     

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/ShaderCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/Resources/Shader/ShaderResource.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Texture/TextureResource.h>
#include <Renderer/Resources/Mesh/VAOResource.h>
#include <Renderer/Deferred/DeferredFrontend.h>

#include "SkinComponent.h"

namespace MoonGlare {
namespace Component {

RegisterComponentID<DirectAnimationComponent> DirectAnimationComponentReg("DirectAnimation");

DirectAnimationComponent::DirectAnimationComponent(SubsystemManager * Owner)
    : TemplateStandardComponent(Owner)
    , m_TransformComponent(nullptr) {}

DirectAnimationComponent::~DirectAnimationComponent() {}

//------------------------------------------------------------------------------------------

void DirectAnimationComponent::RegisterScriptApi(ApiInitializer & root) {
}

//------------------------------------------------------------------------------------------

bool DirectAnimationComponent::Initialize() {
    //	m_Array.MemZeroAndClear();
    //	m_Array.fill(MeshEntry());

    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }

    return true;
}

bool DirectAnimationComponent::Finalize() {
    return true;
}

void DirectAnimationComponent::Step(const Core::MoveConfig &conf) {
    auto *rf = GetManager()->GetWorld()->GetRendererFacade();
    auto &layers = conf.m_BufferFrame->GetCommandLayers();
    auto &m = conf.m_BufferFrame->GetMemory();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::Controll>();
    auto &q = Queue;

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
        auto &item = m_Array[i];

        if (!item.m_Flags.m_Map.m_Valid) {
            continue;
        }

        if (item.m_Flags.m_Map.m_Playing) {
            item.m_LocalTime += conf.timeDelta;
        }

        if (!item.m_Flags.m_Map.m_Visible) {
            continue;
        }

        float FramePosition = item.m_LocalTime * item.m_FramesPerSecond;
        auto framecount = item.m_EndFrame - item.m_FirstFrame + 1;
        FramePosition -= static_cast<unsigned>(FramePosition / framecount) * framecount;
        FramePosition += item.m_FirstFrame;

        if (item.m_Flags.m_Map.m_Playing) {
            item.Calculate(FramePosition);//(item.m_LocalTime / 1.5f);

            for (size_t index = 0; index < item.m_LocalTransformations.size(); ++index) {
                auto tri = m_TransformComponent->GetComponentIndex(item.bones[index]);
                if (tri != ComponentIndex::Invalid) {

                    auto &local = item.m_LocalTransformations[index];
                    m_TransformComponent->SetPosition(tri, emath::MathCast<emath::fvec3>(local.position));
                    m_TransformComponent->SetScale(tri, emath::MathCast<emath::fvec3>(local.scale));
                    m_TransformComponent->SetRotation(tri, emath::MathCast<emath::Quaternion>(local.quaternion));
                } else
                    __debugbreak();
            }
        }
    }
}

bool DirectAnimationComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    x2c::Component::DirectAnimationComponent::DirectAnimationEntry_t dae;
    dae.ResetToDefault();

    if (!reader.Read(dae)) {
        AddLogf(Error, "Failed to read DirectAnimationEntry entry!");
        return false;
    }

    StarVFS::ByteTable data;
    if (!GetFileSystem()->OpenFile(dae.m_AnimationFile, DataPath::URI, data)) {
        AddLogf(Error, "Unable to load model: %s", dae.m_AnimationFile.c_str());
        return false;
    }
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFileFromMemory(
        data.get(), data.size(),
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        //aiProcess_GenUVCoords | 
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        //aiProcess_SortByPType
        0
        ,
        strrchr(dae.m_AnimationFile.c_str(), '.'));

    if (!scene) {
        AddLog(Error, "Unable to to load model file[Name:'" << dae.m_AnimationFile.c_str() << "']. Error: " << importer.GetErrorString());
        return false;
    }

    scene = importer.GetOrphanedScene();

    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();
    entry.Reset();

    entry.m_Flags.m_Map.m_Loop = dae.m_Loop;
    entry.m_Flags.m_Map.m_Playing = true;
    entry.m_EndFrame = dae.m_EndFrame;
    entry.m_FirstFrame = dae.m_FirstFrame;
    entry.m_FramesPerSecond = dae.m_FramesPerSecond;
    entry.m_Flags.m_Map.m_Visible = dae.m_Visible;
    entry.m_CurrentAnimIndex = 2;

    //Load meshes
    {
        using MeshData = DirectAnimationComponentEntry::MeshData;

        std::vector<math::fvec3> verticles;
        std::vector<math::fvec3> normals;
        std::vector<math::fvec2> texCords;
        std::vector<unsigned> indices;

        unsigned NumVertices = 0, NumIndices = 0;
        entry.m_Meshes.resize(scene->mNumMeshes);
        for (unsigned i = 0; i < entry.m_Meshes.size(); i++) {
            MeshData &meshd = entry.m_Meshes[i];
            meshd.NumIndices = scene->mMeshes[i]->mNumFaces * 3;
            meshd.VertexCount = scene->mMeshes[i]->mNumVertices;
            meshd.BaseVertex = NumVertices;
            meshd.BaseIndex = NumIndices;
            meshd.ElementMode = GL_TRIANGLES;// Graphic::Flags::fTriangles;

            NumVertices += meshd.VertexCount;
            NumIndices += meshd.NumIndices;
        }

        verticles.resize(NumVertices);
        normals.resize(NumVertices);
        texCords.resize(NumVertices);

        entry.m_BoneIds.resize(NumVertices, math::uvec4(0));
        entry.m_BoneWeights.resize(NumVertices, math::fvec4(0));
        entry.m_RunTimeMesh.resize(NumVertices);
        entry.m_RunTimeMeshNormals.resize(NumVertices);

        std::vector<unsigned char> BoneIdAllocation;
        BoneIdAllocation.resize(NumVertices, 0);

        indices.reserve(NumIndices);

        unsigned boneoffset = 0;
        for (unsigned i = 0; i < entry.m_Meshes.size(); ++i) {
            const aiMesh* mesh = scene->mMeshes[i];
            MeshData& meshd = entry.m_Meshes[i];
            //meshd.MaterialIndex = mesh->mMaterialIndex;
            meshd.BoneCount = mesh->mNumBones;

            unsigned BaseVertex = meshd.BaseVertex;
            for (unsigned j = 0; j < meshd.BoneCount; ++j) {
                const auto bone = mesh->mBones[j];
                //AddLogf(Hint, "DAC Verts:%4d Bone:%2d weights:%3d Name:%s", mesh->mNumVertices , j, bone->mNumWeights, bone->mName.data);
                for (unsigned k = 0; k < bone->mNumWeights; ++k) {
                    auto &VertexWeight = bone->mWeights[k];

                    auto vertexid = BaseVertex + VertexWeight.mVertexId;
                    auto localboneid = BoneIdAllocation[vertexid]++;

                    entry.m_BoneIds[vertexid][localboneid] = j + boneoffset;
                    entry.m_BoneWeights[vertexid][localboneid] = VertexWeight.mWeight;
                }
            }
            boneoffset += meshd.BoneCount;

            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                aiVector3D &vertex = mesh->mVertices[j];
                aiVector3D &normal = mesh->mNormals[j];

                auto vid = BaseVertex + j;

                if (mesh->mTextureCoords[0]) {
                    aiVector3D &UVW = mesh->mTextureCoords[0][j]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
                    math::vec2 uv(UVW.x, UVW.y);

                    texCords[vid] = uv;
                } else {
                    texCords[vid] = math::vec2();
                }

                verticles[vid] = math::vec3(vertex.x, vertex.y, vertex.z);
                normals[vid] = math::vec3(normal.x, normal.y, normal.z);
            }

            for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
                aiFace *f = &mesh->mFaces[k];
                THROW_ASSERT(f->mNumIndices == 3, 0);
                for (unsigned j = 0; j < 3; ++j) {
                    indices.push_back(f->mIndices[j]);
                }
            }
        }

        entry.m_Mesh.swap(verticles);
        entry.m_MeshNormals.swap(normals);
        entry.m_uvs.swap(texCords);
        entry.m_indices.swap(indices);

        verticles.resize(entry.m_Mesh.size(), math::fvec3());
        normals.resize(entry.m_Mesh.size(), math::fvec3());

    }

    entry.m_Scene = scene;
    entry.Load();

    entry.m_Owner = owner;

    entry.m_Flags.m_Map.m_Valid = true;

    //entry.bones

    auto *anim = scene->mAnimations[entry.m_CurrentAnimIndex];
    for (size_t bone = 0; bone < anim->mNumChannels; ++bone) {
        auto e = m_TransformComponent->FindChildByName(owner, anim->mChannels[bone]->mNodeName.data);
        if (!e.has_value()) {
            __debugbreak();
        }
        entry.bones[bone] = e.value_or(Entity{});
    }

    return true;
    }

bool DirectAnimationComponent::Create(Entity owner) {
    return false;
}

} //namespace Component 
} //namespace MoonGlare 
