#include <pch.h>

#define NEED_VAO_BUILDER     

#include <nfMoonGlare.h>

#include "SkinComponent.h"
#include "SkinSystem.h"

#include "TransformComponent.h"

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
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Deferred/DeferredFrontend.h>

#include <Core/Component/SubsystemManager.h>

namespace MoonGlare::Component {

SkinSystem::SkinSystem(iSubsystemManager * Owner) : iSubsystem(), subsystemManager(Owner), transformComponent(nullptr){
}

SkinSystem::~SkinSystem() {
}

//------------------------------------------------------------------------------------------

bool SkinSystem::Initialize() {
    auto *ssm = (Core::Component::SubsystemManager*)subsystemManager;
    transformComponent = ssm->GetComponent<TransformComponent>();
    if (!transformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }
    
    return true;
}

#if  0

//------------------------------------------------------------------------------------------

void MeshComponent::HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event) {
    auto index = m_EntityMapper.GetIndex(event.entity);
    if (index >= m_Array.Allocated())
        return;

    m_Array[index].m_Flags.m_Map.m_Valid = false;
    m_EntityMapper.SetIndex(event.entity, ComponentIndex::Invalid);
}

//------------------------------------------------------------------------------------------
#endif

void SkinSystem::Step(const SubsystemUpdateData &xconf) {
    assert(transformComponent);

    auto *ssm = (Core::Component::SubsystemManager*)subsystemManager;
    auto *rf = ssm->GetWorld()->GetRendererFacade();
    const Core::MoveConfig &conf = (const Core::MoveConfig &)xconf;
    auto &layers = conf.m_BufferFrame->GetCommandLayers();
    auto &m = conf.m_BufferFrame->GetMemory();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::Controll>();
    auto &q = Queue;

    subsystemManager->GetComponentArray().Visit<SkinComponent>([this, &conf, &q, &m, rf, ssm](Entity owner, SkinComponent& item) {
        if (item.meshData == nullptr) {
            auto &mm = ssm->GetWorld()->GetRendererFacade()->GetResourceManager()->GetMeshManager();
            if (!mm.GetMeshData(item.meshHandle) || mm.GetMeshData(item.meshHandle)->ready == false)
                return;
            auto *mdataptr = mm.GetMeshData(item.meshHandle);
            item.meshData = mdataptr;
        }

        if (item.meshData == nullptr) {
            return;
        }

        glm::fvec3 *verticles = m.Clone(item.meshData->verticles, item.meshData->vertexCount);
        glm::fvec3 *normals = m.Clone(item.meshData->normals, item.meshData->vertexCount);
        glm::fvec3 *tangents = m.Clone(item.meshData->tangents, item.meshData->vertexCount);

        //animate mesh
        glm::fmat4 matrices[64];

        for (size_t i = 0; i < item.validBones; ++i) {
            auto e = item.bones[i];
            auto ti = transformComponent->GetComponentIndex(e);
            if (ti == ComponentIndex::Invalid) {
                __debugbreak();
                continue;
            }

            const emath::Transform &tr = transformComponent->GetTransform(ti);
            const glm::fmat4 &offset = item.meshData->boneMatrices[i];

            matrices[i] = emath::MathCast<math::mat4>(tr.matrix()) * offset;
        }
        auto parent = transformComponent->GetOwner(owner);

        for (unsigned vid = 0; vid < item.meshData->vertexCount; ++vid) {
            const auto in = item.meshData->verticles[vid];
            const auto innormal = item.meshData->normals[vid];
            const auto intangent = item.meshData->tangents[vid];
            const auto inboneids = item.meshData->vertexBones[vid];
            const auto inbonew = item.meshData->vertexBoneWeights[vid];

            glm::vec4 out4(0);
            glm::vec4 out4normal(0);
            glm::vec4 out4tangent(0);

            glm::vec4 in4(in, 1);
            glm::vec4 in4normal(innormal, 0);
            glm::vec4 in4tangent(intangent, 0);

            for (unsigned bid = 0; bid < 4; ++bid) {
                if (inboneids[bid] == Renderer::MeshSource::InvalidBoneIndex)
                    break;

                auto boneindex = inboneids[bid];
                const glm::fmat4 &transform = matrices[boneindex];

                auto w = inbonew[bid];
                out4 += (transform * in4) * w;
                out4normal += (transform * in4normal) * w;
                out4tangent += (transform * in4tangent) * w;
            }

            verticles[vid] = out4;
            normals[vid] = out4normal;
            tangents[vid] = out4tangent;
        }

        using ichannels = Renderer::Configuration::VAO::InputChannels;

        auto vaob = rf->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, item.vaoHandle, true);
        vaob.BeginDataChange();

        vaob.CreateChannel(ichannels::Vertex);
        vaob.SetChannelData<float, 3>(ichannels::Vertex, (const float*)verticles, item.meshData->vertexCount, true);

        vaob.CreateChannel(ichannels::Normals);
        vaob.SetChannelData<float, 3>(ichannels::Normals, (const float*)normals, item.meshData->vertexCount, true);

        vaob.CreateChannel(ichannels::Tangents);
        vaob.SetChannelData<float, 3>(ichannels::Tangents, (const float*)tangents, item.meshData->vertexCount, true);

        vaob.CreateChannel(ichannels::Texture0);
        vaob.SetChannelData<float, 2>(ichannels::Texture0, (const float*)item.meshData->UV0, item.meshData->vertexCount, true);

        vaob.CreateChannel(ichannels::Index);
        vaob.SetIndex(ichannels::Index, (const uint32_t*)item.meshData->index, item.meshData->indexCount, true);

        vaob.EndDataChange();
        vaob.UnBindVAO();

        if (item.vaoHandle.deviceHandle && *item.vaoHandle.deviceHandle) {//dirty valid check
            conf.deffered->DrawElements(
                emath::Transform::Identity().matrix(),
                item.vaoHandle,
                { item.meshData->indexCount, 0, 0, GL_UNSIGNED_INT, GL_TRIANGLES },
                item.materialHandle
            );
        }
    });
}

//------------------------------------------------------------------------------------------


}
