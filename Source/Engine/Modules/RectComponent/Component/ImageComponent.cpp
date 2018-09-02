#include <pch.h>
#include <nfMoonGlare.h>
#include "../nfGUI.h"

#define NEED_MATERIAL_BUILDER     
#define NEED_VAO_BUILDER     

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
#include <Renderer/Resources/Shader/ShaderResource.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Texture/TextureResource.h>
#include <Renderer/Resources/Mesh/VAOResource.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <ImageComponent.x2c.h>

#include "ImageComponent.h"
#include "Core/Engine.h"

namespace MoonGlare::GUI::Component {

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<ImageComponent, ImageComponentEntry> ImageComponentTypeInfo;
RegisterComponentID<ImageComponent> ImageComponentIDReg("Image");

//---------------------------------------------------------------------------------------

ImageComponent::ImageComponent(SubsystemManager *Owner) 
        : TemplateStandardComponent(Owner)
{
    m_RectTransform = nullptr;

    //memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
    m_Array.fill(ImageComponentEntry());
    m_Array.ClearAllocation();
}

ImageComponent::~ImageComponent() {
}

//---------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer ImageComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
    .beginClass<ImageComponentEntry>("cImageComponentEntry")
        .addProperty("Color", &ImageComponentEntry::GetColor, &ImageComponentEntry::SetColor)
        .addProperty("Speed", &ImageComponentEntry::GetSpeed, &ImageComponentEntry::SetSpeed)
        .addProperty("Position", &ImageComponentEntry::GetPosition, &ImageComponentEntry::SetPosition)
    .endClass()
    ;
}

//---------------------------------------------------------------------------------------

bool ImageComponent::Initialize() {
    m_RectTransform = GetManager()->GetComponent<RectTransformComponent>();
    if (!m_RectTransform) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }

    auto &shres = GetManager()->GetWorld()->GetRendererFacade()->GetResourceManager()->GetShaderResource();
    shres.Load(m_ShaderHandle, "GUI");

    return true;
}

bool ImageComponent::Finalize() {
    return true;
}

//---------------------------------------------------------------------------------------

void ImageComponent::Step(const Core::MoveConfig & conf) {
    auto &layers = conf.m_BufferFrame->GetCommandLayers();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::GUI>();
    auto &q = Queue;

    auto &shres = conf.m_BufferFrame->GetResourceManager()->GetShaderResource();
    auto shb = shres.GetBuilder(q, m_ShaderHandle);
    using Uniform = GUIShaderDescriptor::Uniform;

    shb.Bind();
    shb.Set<Uniform::CameraMatrix>(m_RectTransform->GetCamera().GetProjectionMatrix());

    using namespace Renderer;
    q.MakeCommand<Commands::SetViewport>(0, 0, (int)conf.m_ScreenSize[0], (int)conf.m_ScreenSize[1]);
    q.MakeCommand<Commands::DepthMask>((GLboolean)GL_TRUE);
    q.MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
    q.MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
    q.MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
    q.MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);

    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

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
        
        item.Update(conf.timeDelta, *rtentry);
        
        if (!item.m_DrawEnabled)
            continue;

        Renderer::Commands::CommandKey key{ rtentry->m_Z };

        shb.SetMaterial(item.material, key);

        shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(item.m_ImageMatrix), key);
        shb.Set<Uniform::BaseColor>(emath::MathCast<emath::fvec4>(item.m_Color), key);
        shb.Set<Uniform::TileMode>(emath::ivec2(0, 0), key);

        auto cnt = item.m_FrameCount;
        shb.Set<Uniform::FrameCount>(emath::ivec2(cnt[0], cnt[1]), key);
        auto uframe = item.GetFrameIndex();
        shb.Set<Uniform::FrameIndex>(uframe, key);
        
        Queue.MakeCommandKey<Renderer::Commands::VAOBindResource>(key, item.vaoHandle.deviceHandle);

        auto arg = Queue.PushCommand<Renderer::Commands::VAODrawTrianglesBaseVertex>(key);
        arg->m_NumIndices = 6;
        arg->m_IndexValueType = GL_UNSIGNED_BYTE;
        arg->m_BaseIndex = 0;
        arg->m_BaseVertex = 0;
    }

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "TransformComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        TrivialReleaseElement(LastInvalidEntry);
    }
}

//---------------------------------------------------------------------------------------

bool ImageComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }
    auto &entry = m_Array[index];
    entry.Reset();

    entry.m_Owner = owner;

    x2c::Component::ImageComponent::ImageEntry_t ie;
    ie.ResetToDefault();
    if (!reader.Read(ie)) {
        AddLog(Error, "Failed to read ImageEntry!");
        return false;
    }

    entry.m_Speed = ie.m_Speed;
    entry.m_ScaleMode = ie.m_ScaleMode;
    entry.m_Color = ie.m_Color;
    entry.m_Flags.m_Map.m_Active = ie.m_Active;

    auto s = GetManager()->GetWorld()->GetRendererFacade()->GetContext()->GetSizef();

    entry.Load(ie.m_TextureURI, ie.m_FrameStripCount, m_RectTransform->IsUniformMode(), s);

    if (ie.m_FrameCount == 0)
        ie.m_FrameCount = ie.m_FrameStripCount[0] * ie.m_FrameStripCount[1];

    entry.m_StartFrame = ie.m_StartFrame;
    entry.m_EndFrame = ie.m_StartFrame + ie.m_FrameCount;
    entry.m_FrameCount = ie.m_FrameStripCount;
    entry.m_Position = entry.m_StartFrame;

    auto *rtentry = m_RectTransform->GetEntry(entry.m_Owner);
    if (rtentry) {
        entry.Update(0.0f, *rtentry);
    } else {
        //TODO:??
    }

    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_Dirty = true;
    m_EntityMapper.SetIndex(owner, index);
    return true;
}

bool ImageComponentEntry::Load(const std::string &fileuri, math::uvec2 FrameStripCount, bool Uniform, const emath::fvec2 &ScreenSize) {

    auto *e = Core::GetEngine();
    auto *rf = e->GetWorld()->GetRendererFacade();
    auto *resmgr = rf->GetResourceManager();

    Renderer::MaterialTemplate matT;
    matT.diffuseColor = { 1,1,1,1 };
    matT.diffuseMap.enabled = true;
    matT.diffuseMap.texture = fileuri;

    material = resmgr->GetMaterialManager().CreateMaterial(matT.diffuseMap.texture, matT);

    auto matH = material;

    //FIXME: ugly!
    rf->GetAsyncLoader()->QueueTask(std::make_shared<Renderer::FunctionalAsyncTask>(
        [this, rf, Uniform, matH, ScreenSize, FrameStripCount](Renderer::ResourceLoadStorage &storage) {

        auto *matPtr = rf->GetResourceManager()->GetMaterialManager().GetMaterial(matH);

        auto TextureSize = emath::MathCast<math::fvec2>(rf->GetResourceManager()->GetTextureResource().GetSize(matPtr->mapTexture[0]));
        if (TextureSize[0] <= 0 || TextureSize[1] <= 0)
            throw Renderer::FunctionalAsyncTask::RetryLater{}; //TODO: this may be infinite if texture is invalid

        math::vec2 FrameSize;
        if (Uniform) {
            auto screen = emath::MathCast<math::fvec2>(ScreenSize);
            float Aspect = screen[0] / screen[1];
            FrameSize = TextureSize;
            FrameSize /= math::vec2(FrameStripCount);
            FrameSize /= screen;
            FrameSize.x *= Aspect;
            FrameSize *= 2.0f;
        } else {
            FrameSize = TextureSize;
        }
        m_FrameSize = FrameSize;
        m_Flags.m_Map.m_Dirty = true;

        std::array<glm::fvec3, 4> Vertexes = {
            glm::fvec3(0, FrameSize[1], 0),
            glm::fvec3(FrameSize[0], FrameSize[1], 0),
            glm::fvec3(FrameSize[0], 0, 0),
            glm::fvec3(0, 0, 0),
        };

        static const std::array<glm::fvec2, 4> UVs = {
            glm::fvec2(0, 0),
            glm::fvec2(1, 0),
            glm::fvec2(1, 1),
            glm::fvec2(0, 1),
        };

        auto &m = storage.m_Memory.m_Allocator;
        auto &q = storage.m_Queue;

        using ichannels = Renderer::Configuration::VAO::InputChannels;

        auto vaob = rf->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, vaoHandle, true);
        vaob.BeginDataChange();

        vaob.CreateChannel(ichannels::Vertex);
        vaob.SetChannelData<float, 3>(ichannels::Vertex, (const float*)m.Clone(Vertexes), Vertexes.size());

        vaob.CreateChannel(ichannels::Texture0);
        vaob.SetChannelData<float, 2>(ichannels::Texture0, (const float*)m.Clone(UVs), UVs.size());

        vaob.CreateChannel(ichannels::Index);
        static constexpr std::array<uint8_t, 6> IndexTable = { 0, 1, 2, 0, 2, 3, };
        vaob.SetIndex(ichannels::Index, IndexTable);

        vaob.EndDataChange();
        vaob.UnBindVAO();

        m_DrawEnabled = true;
    }));

    return true;
}

//---------------------------------------------------------------------------------------

void ImageComponentEntry::Update(float TimeDelta, RectTransformComponentEntry &rectTransform) {
    if (m_Speed > 0) {
        m_Position += m_Speed * TimeDelta;
        if ((unsigned)m_Position >= m_EndFrame) {
            float cnt = m_EndFrame - m_StartFrame;
            if (cnt == 0)
                cnt = 1;
            int mult = static_cast<int>(m_Position / cnt);
            m_Position -= static_cast<float>(mult) * cnt;
        }
    }

    if (m_Flags.m_Map.m_Dirty || m_TransformRevision != rectTransform.m_Revision) {
        m_Flags.m_Map.m_Dirty = false;
        m_TransformRevision = rectTransform.m_Revision;

        math::vec3 Pos, Scale;

        switch (m_ScaleMode) {
        case ImageScaleMode::None:
            m_ImageMatrix = rectTransform.m_GlobalMatrix;
            return;
        case ImageScaleMode::Center: {
            auto halfparent = rectTransform.m_ScreenRect.GetSize() / 2.0f;
            auto halfsize = m_FrameSize / 2.0f;
            Pos = math::vec3(halfparent - halfsize, 0.0f);
            Scale = math::vec3(1.0f);
            break;
        }
        case ImageScaleMode::ScaleToFit:{
            Point ratio = rectTransform.m_ScreenRect.GetSize() / m_FrameSize;
            Scale = math::vec3(ratio, 1.0f);
            Pos = math::vec3(0);
            break;
        }
        case ImageScaleMode::ScaleProportional: 
        case ImageScaleMode::ScaleProportionalCenter: {
            auto parentsize = rectTransform.m_ScreenRect.GetSize();
            auto &framesize = m_FrameSize;
            Point ratio = parentsize / framesize;
            float minration = math::min(ratio.x, ratio.y);
            Scale = math::vec3(minration, minration, 1.0f);
            if (m_ScaleMode == ImageScaleMode::ScaleProportionalCenter) {
                auto halfparent = rectTransform.m_ScreenRect.GetSize() / 2.0f;
                auto halfsize = (m_FrameSize * math::vec2(minration)) / 2.0f;
                Pos = math::vec3(halfparent - halfsize, 0.0f);
            } else {
                Pos = math::vec3(0.0f);
            }
            break;
        }
        default:
            LogInvalidEnum(m_ScaleMode);
            return;
        }
        m_ImageMatrix = rectTransform.m_GlobalMatrix * glm::scale(glm::translate(math::mat4(), Pos), Scale);
    }
}

}
