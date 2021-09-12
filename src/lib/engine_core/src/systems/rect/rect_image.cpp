#include "rect_image.hpp"
#include "component/global_matrix.hpp"
#include "component/rect/rect_image.hpp"
#include "component/rect/rect_transform.hpp"
#include "ecs/component_array.hpp"
#include "renderer/frame_sink_interface.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
// #include "component/local_matrix.hpp"
// #include "renderer/resources.hpp"

namespace MoonGlare::Systems::Rect {

using namespace Component;
using namespace Component::Rect;

using iFrameSink = Renderer::iFrameSink;

RectImageSystem::RectImageSystem(const ECS::SystemCreateInfo &create_info,
                                 SystemConfiguration config_data)
    : SystemBase(create_info, config_data) {
}

void RectImageSystem::DoStep(double time_delta) {

    // auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::GUI>();

    // auto &shres = conf.m_BufferFrame->GetResourceManager()->GetShaderResource();
    // auto shb = shres.GetBuilder(q, m_ShaderHandle);
    // using Uniform = GUIShaderDescriptor::Uniform;
    // using Sampler = GUIShaderDescriptor::Sampler;

    // shb.Bind();
    // shb.Set<Uniform::CameraMatrix>(m_RectTransform->GetCamera().GetProjectionMatrix());

    // using namespace Renderer;
    // q.MakeCommand<Commands::SetViewport>(0, 0, (int)conf.m_ScreenSize[0], (int)conf.m_ScreenSize[1]);
    // q.MakeCommand<Commands::DepthMask>((GLboolean)GL_TRUE);
    // q.MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
    // q.MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
    // q.MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
    // q.MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);

    GetComponentArray()->Visit<RectImage, RectTransform, GlobalMatrix>( //
        [&](const RectImage &image, const RectTransform &rect, const GlobalMatrix &global_matrix) {
            // item.Update(conf.timeDelta, *rtentry);

            auto element_buffer = GetFrameSink()->ReserveElements(iFrameSink::ElementReserve{
                .index_count = 6,
                .vertex_count = 4,
                .texture0_count = 4,
            });

            if (element_buffer.index_buffer == nullptr || element_buffer.vertex_buffer == nullptr ||
                element_buffer.texture0_buffer == nullptr) {
                AddLogf(Warning, "Failed to allocate element buffer");
                return;
            }

            // Renderer::Commands::CommandKey key{ rtentry->m_Z };
            // //shb.SetMaterial(item.material, key);
            // shb.Set<Sampler::DiffuseMap>(item.material.deviceHandle->mapTexture[(Material::MapType)0], key);
            // shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(item.m_ImageMatrix), key);
            // shb.Set<Uniform::BaseColor>(emath::MathCast<emath::fvec4>(item.m_Color), key);
            // shb.Set<Uniform::TileMode>(emath::ivec2(0, 0), key);
            // Queue.MakeCommandKey<Renderer::Commands::VAOBindResource>(key, item.vaoHandle.deviceHandle);

            auto w = rect.size[0];
            auto h = rect.size[1];
            std::array<math::fvec3, 4> points = {
                math::fvec3(w * 0, h * 0, 0.0f),
                math::fvec3(w * 1.0f, h * 0, 0.0f),
                math::fvec3(w * 1.0f, h * 1.0f, 0.0f),
                math::fvec3(w * 0, h * 1.0f, 0.0f),
            };
            for (uint16_t i = 0; i < points.size(); ++i) {
                element_buffer.vertex_buffer[i] = points[i];
            }

            std::array<math::fvec2, 4> tex_points = {
                math::fvec2(0.0f, 0.0f),
                math::fvec2(1.0f, 0.0f),
                math::fvec2(1.0f, 1.0f),
                math::fvec2(0.0f, 1.0f),
            };

            for (uint16_t i = 0; i < tex_points.size(); ++i) {
                element_buffer.texture0_buffer[i] = tex_points[i];
            }

            static const std::array<uint16_t, 6> indexes = {
                0, 1, 2, 0, 2, 3,
            };
            for (uint16_t i = 0; i < indexes.size(); ++i) {
                element_buffer.index_buffer[i] = indexes[i];
            }

            auto req = iFrameSink::ElementRenderRequest{
                .position_matrix = global_matrix.transform.matrix(), //math::fmat4::Identity(),
                .element_mode = GL_TRIANGLES,
                .index_count = 6,
                .shader_handle = image.shader_handle.loaded_handle,
                .texture_handle = image.image_handle.loaded_handle,
            };

            GetFrameSink()->SubmitElements(element_buffer, req);
        });
}

} // namespace MoonGlare::Systems::Rect

#if 0

class ImageComponent
    : public TemplateStandardComponent<ImageComponentEntry, SubSystemId::Image> {
public:
    ImageComponent(SubsystemManager *Owner);
    virtual ~ImageComponent();
    virtual bool Initialize() override;
    virtual bool Finalize() override;
    virtual void Step(const Core::MoveConfig &conf) override;
    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
protected:
    RectTransformComponent *m_RectTransform;
    Renderer::ShaderResourceHandle<GUIShaderDescriptor> m_ShaderHandle;
};

namespace MoonGlare::GUI::Component {

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
    matT.diffuseMap.enabled = true;
    matT.diffuseMap.texture = fileuri;

    material = resmgr->GetMaterialManager().CreateMaterial(matT.diffuseMap.texture, matT);

    auto matH = material;

    //FIXME: ugly!
    rf->GetAsyncLoader()->QueueTask(std::make_shared<Renderer::FunctionalAsyncTask>(
        [this, rf, Uniform, matH, ScreenSize, FrameStripCount](Renderer::ResourceLoadStorage &storage) {

        auto *matPtr = rf->GetResourceManager()->GetMaterialManager().GetMaterial(matH);

        auto TextureSize = emath::MathCast<math::fvec2>(rf->GetResourceManager()->GetTextureResource().GetSize(matPtr->mapTexture[Renderer::Material::MapType::Diffuse]));
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

        std::vector<glm::fvec3> Vertexes;
        std::vector<glm::fvec2> UVs;

        unsigned FrameCount = FrameStripCount[0] * FrameStripCount[1];

        Vertexes.reserve(FrameCount * 4);
        UVs.reserve(FrameCount * 4);
        UVs.reserve(FrameCount * 4);

        math::vec2 fu = math::vec2(1.0f) / math::vec2(FrameStripCount);

        for (unsigned y = 0; y < FrameStripCount[1]; ++y)
            for (unsigned x = 0; x < FrameStripCount[0]; ++x) {
                unsigned frame = y * FrameStripCount[0] + x;
                if (frame > FrameCount)
                    continue;

                Vertexes.push_back(glm::fvec3(0, FrameSize[1], 0));
                Vertexes.push_back(glm::fvec3(FrameSize[0], FrameSize[1], 0));
                Vertexes.push_back(glm::fvec3(FrameSize[0], 0, 0));
                Vertexes.push_back(glm::fvec3(0, 0, 0));

                //Graphic::NormalVector Normals;
                float w1 = fu[0] * (float)x;
                float h1 = fu[1] * (float)y;
                float w2 = w1 + fu[0];
                float h2 = h1 + fu[1];

                UVs.push_back(glm::fvec2(w1, 1.0f - h2));
                UVs.push_back(glm::fvec2(w2, 1.0f - h2));
                UVs.push_back(glm::fvec2(w2, 1.0f - h1));
                UVs.push_back(glm::fvec2(w1, 1.0f - h1));
            }

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
        m_ImageMatrix = rectTransform.m_GlobalMatrix * glm::scale(glm::translate(glm::identity<glm::fmat4>(), Pos), Scale);
    }
}

}

#endif