#include <pch.h>
#include <MoonGlare.h>

#include "nfGUI.h"
#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include "Component/RectTransformComponent.h"
#include "RectTransformDebugDraw.h"

#include <Renderer/Frame.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/iContext.h>

#include "GUIShader.h"

namespace MoonGlare::GUI::Component {

RectTransformComponent::RectTransformDebugDraw::RectTransformDebugDraw() {}

RectTransformComponent::RectTransformDebugDraw::~RectTransformDebugDraw() {}

void RectTransformComponent::RectTransformDebugDraw::DebugDraw(const Core::MoveConfig &conf, RectTransformComponent *Component) {
    auto frame = conf.m_BufferFrame;

    if (!ready) {
        auto &shres = frame->GetResourceManager()->GetShaderResource();
        if (!shres.Load<GUIShaderDescriptor>(shaderHandle, GUIShaderDescriptor::ResourceName)) {
            AddLogf(Error, "Failed to load shader");
            return;
        }

        ready = true;
        return;
    }

    using namespace Renderer;
    using Uniform = GUIShaderDescriptor::Uniform;

    Renderer::VAOResourceHandle vao;
    if (!frame->AllocateFrameResource(vao))
        return;

    auto &layers = frame->GetCommandLayers();
    auto &qgui = layers.Get<Renderer::Configuration::FrameBuffer::Layer::GUI>();

    auto &q = *frame->AllocateSubQueue();
    qgui.PushQueue(&q, Renderer::Commands::CommandKey::Max());
    auto &shres = frame->GetResourceManager()->GetShaderResource();

    auto key = Renderer::Commands::CommandKey::Max();
    //qgui.PushQueue(qptr, basekey);

    auto shb = shres.GetBuilder<GUIShaderDescriptor>(q, shaderHandle);

    shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(math::mat4()), key);
    shb.Set<Uniform::TileMode>(emath::ivec2(0, 0), key);
    shb.Set<Uniform::BaseColor>(emath::fvec4(1, 1, 1, 1), key);
    //  shb.Bind(key);

    //m_Shader->SetWorldMatrix(q, basekey, emath::fmat4::Identity(), m_Camera.GetProjectionMatrix());
    //m_Shader->SetColor(q, basekey, math::fvec4(1));

    q.MakeCommandKey<Commands::Texture2DBind>(key, Renderer::Device::InvalidTextureHandle);
   // q.MakeCommandKey<Commands::Enable>(key, (GLenum)GL_BLEND);
    q.MakeCommandKey<Commands::Disable>(key, (GLenum)GL_CULL_FACE);
    q.MakeCommandKey<Commands::Disable>(key, (GLenum)GL_DEPTH_TEST);
    q.MakeCommandKey<Commands::EnterWireFrameMode>(key);

    auto *vertexes = frame->GetMemory().Allocate<math::fvec3>(4 * Component->m_Array.Allocated());
    auto *indextable = frame->GetMemory().Allocate<uint16_t>(5 * Component->m_Array.Allocated());
    uint16_t vertexindex = 0;
    uint16_t indexindex = 0;

    for (size_t i = 1; i < Component->m_Array.Allocated(); ++i) {//ignore root entry
        auto &item = Component->m_Array[i];
        if (!item.m_Flags.m_Map.m_Valid) {
            continue;
        }

        auto &r = item.m_ScreenRect;
        indextable[indexindex++] = vertexindex;
        vertexes[vertexindex++] = math::vec3(r.LeftTop.x, r.LeftTop.y, 0.0f);
        indextable[indexindex++] = vertexindex;
        vertexes[vertexindex++] = math::vec3(r.LeftTop.x, r.RightBottom.y, 0.0f);
        indextable[indexindex++] = vertexindex;
        vertexes[vertexindex++] = math::vec3(r.RightBottom.x, r.RightBottom.y, 0.0f);
        indextable[indexindex++] = vertexindex;
        vertexes[vertexindex++] = math::vec3(r.RightBottom.x, r.LeftTop.y, 0.0f);
    }

    auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, vao, true);

    vaob.BeginDataChange();
    using ichannels = Renderer::Configuration::VAO::InputChannels;

    vaob.CreateChannel(ichannels::Vertex);
    vaob.SetChannelData<float, 3>(ichannels::Vertex, &vertexes[0][0], vertexindex);

    vaob.CreateChannel(ichannels::Texture0);
    vaob.SetChannelData<float, 2>(ichannels::Texture0, nullptr, 0);

    //vaob.CreateChannel(ichannels::Index);
    //vaob.SetIndex(ichannels::Index, indextable, indexindex);

    vaob.EndDataChange();

    q.MakeCommand<Renderer::Commands::VAODrawArrays>((GLenum)GL_QUADS, (GLint)0, indexindex);

    vaob.UnBindVAO();

    q.MakeCommand<Renderer::Commands::LeaveWireFrameMode>();

}

} //namespace MoonGlare::GUI::Component 
