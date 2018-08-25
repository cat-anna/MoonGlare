/*
  * Generated by cppsrc.sh
  * On 2016-09-18 19:00:53,20
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>

#define NEED_VAO_BUILDER

#include <MoonGlare.h>
#include "../nfGUI.h"

#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include "RectTransformComponent.h"

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/ShaderCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Frame.h>
#include <Renderer/Renderer.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <RectTransformComponent.x2c.h>
#include <PanelComponent.x2c.h>

#include "PanelComponent.h"

namespace MoonGlare {
namespace GUI {
namespace Component {

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<PanelComponent, PanelComponentEntry> PanelComponentTypeInfo;
RegisterComponentID<PanelComponent> PanelComponentIDReg("Panel");

//---------------------------------------------------------------------------------------

PanelComponent::PanelComponent(SubsystemManager *Owner) 
        : TemplateStandardComponent(Owner)
{
    //memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
    //	m_Array.fill(PanelComponentEntry());
    m_Array.ClearAllocation();
}

PanelComponent::~PanelComponent() {
}

//---------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer PanelComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
        .beginClass<PanelComponentEntry>("cPanelComponentEntry")
            .addProperty("Color", &PanelComponentEntry::GetColor, &PanelComponentEntry::SetColor)
            .addProperty("Border", &PanelComponentEntry::GetBorder, &PanelComponentEntry::SetBorder)
            .addProperty("TileMode", &PanelComponentEntry::GetTileMode, &PanelComponentEntry::SetTileMode)
        .endClass()
        ;
}

//---------------------------------------------------------------------------------------

bool PanelComponent::Initialize() {
    m_RectTransform = GetManager()->GetComponent<RectTransformComponent>();
    if (!m_RectTransform) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }

    auto &shres = GetManager()->GetWorld()->GetRendererFacade()->GetResourceManager()->GetShaderResource();
    if (!shres.Load(m_ShaderHandle, "GUI")) {
        AddLogf(Error, "Failed to load GUI shader");
        return false;
    }

    return true;
}

bool PanelComponent::Finalize() {
    return true;
}

//---------------------------------------------------------------------------------------

void PanelComponent::Step(const Core::MoveConfig & conf) {
    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

    auto &layers = conf.m_BufferFrame->GetCommandLayers();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::GUI>();
    auto &q = Queue;

    auto &shres = conf.m_BufferFrame->GetResourceManager()->GetShaderResource();
    auto shb = shres.GetBuilder(q, m_ShaderHandle);
    using Uniform = GUIShaderDescriptor::Uniform;

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {
        auto &item = m_Array[i];

        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and continue
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }

        auto *rtentry = m_RectTransform->GetEntry(item.m_Owner);
        if (!rtentry) {
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }

        if (!item.m_Flags.m_Map.m_Active)
            continue;

        if (item.m_TransformRevision == rtentry->m_Revision && !item.m_Flags.m_Map.m_Dirty) {
        } else {

            item.m_Flags.m_Map.m_Dirty = false;
            item.m_TransformRevision = rtentry->m_Revision;

            auto size = rtentry->m_ScreenRect.GetSize();

            math::vec3 Vertexes[4] = {
                math::vec3(0, size[1], 0),
                math::vec3(size[0], size[1], 0),
                math::vec3(size[0], 0, 0),
                math::vec3(0, 0, 0),
            };
            float w1 = 0.0f;
            float h1 = 0.0f;
            float w2 = 1.0f;
            float h2 = 1.0f;
            math::vec2 TexUV[4] = {
                math::vec2(w1, h1),
                math::vec2(w2, h1),
                math::vec2(w2, h2),
                math::vec2(w1, h2),
            };

            {//FIXME: transition to new api is not tested!
                auto &m = conf.m_BufferFrame->GetMemory();

                using ichannels = Renderer::Configuration::VAO::InputChannels;

                auto vaob = conf.m_BufferFrame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, item.vaoHandle, true);
                vaob.BeginDataChange();

                vaob.CreateChannel(ichannels::Vertex);
                vaob.SetChannelData<float, 3>(ichannels::Vertex, (const float*)m.Clone(Vertexes), 4);

                vaob.CreateChannel(ichannels::Texture0);
                vaob.SetChannelData<float, 2>(ichannels::Texture0, (const float*)m.Clone(TexUV), 4);

                vaob.CreateChannel(ichannels::Index);
                static constexpr std::array<uint8_t, 6> IndexTable = { 0, 1, 2, 0, 2, 3, };
                vaob.SetIndex(ichannels::Index, IndexTable);

                vaob.EndDataChange();
                vaob.UnBindVAO();
            }
        }

        Renderer::Commands::CommandKey key{ rtentry->m_Z };

        shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(rtentry->m_GlobalMatrix), key);
        shb.Set<Uniform::BaseColor>(emath::MathCast<emath::fvec4>(item.m_Color), key);
        shb.Set<Uniform::TileMode>(emath::MathCast<emath::ivec2>(item.m_TileMode), key);

        auto ps = rtentry->m_ScreenRect.GetSize();
        shb.Set<Uniform::PanelSize>(emath::MathCast<emath::fvec2>(ps), key);
        shb.Set<Uniform::PanelAspect>(ps[0] / ps[1], key);

        shb.Set<Uniform::Border>(item.m_Border, key);
        shb.Set<Uniform::FrameCount>(emath::ivec2(0,0), key);

//		Queue.PushCommand<Renderer::Commands::Texture2DBind>(key)->m_Texture = item.m_Texture->Handle();
        Queue.MakeCommandKey<Renderer::Commands::VAOBindResource>(key, item.vaoHandle.deviceHandle);

        auto arg = Queue.PushCommand<Renderer::Commands::VAODrawTriangles>(key);
        arg->m_NumIndices = 6;
        arg->m_IndexValueType = GL_UNSIGNED_INT;

    }

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "TransformComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        TrivialReleaseElement(LastInvalidEntry);
    }
}

//---------------------------------------------------------------------------------------

bool PanelComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }
    auto &entry = m_Array[index];
    entry.Reset();

    entry.m_Owner = owner;

    x2c::Component::PanelComponent::PanelEntry_t pe;
    pe.ResetToDefault();
    if (!reader.Read(pe)) {
        AddLog(Error, "Failed to read ImageEntry!");
        return false;
    }

    auto *rtentry = m_RectTransform->GetEntry(entry.m_Owner);
    if (rtentry) {
    } else {
        //TODO:??
    }

    //TODO: Texture loading in PanelComponent
    //if (!GetFileSystem()->OpenTexture(entry.m_Texture, pe.m_TextureURI)) {
    //	AddLog(Error, "Unable to load texture file for panel!");
    //	return false;
    //}
    
    entry.m_Border = pe.m_Border;
    entry.m_Color = pe.m_Color;
    entry.m_TileMode = pe.m_TileMode;
    entry.m_Flags.m_Map.m_Active = pe.m_Active;

    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_Dirty = true;
    m_EntityMapper.SetIndex(owner, index);
    return true;
}

//---------------------------------------------------------------------------------------

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 
