#include <pch.h>
#include <MoonGlare.h>
#include "iFont.h"

#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/RenderInput.h>

#include <Renderer/Frame.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/TextureRenderTask.h>
#include <Renderer/Resources/ResourceManager.h>

#include <Renderer/SimpleFontShaderDescriptor.h>

namespace MoonGlare {
namespace DataClasses {
namespace Fonts {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(iFont);

bool iFont::RenderText(const std::wstring & text, Renderer::Frame * frame, const FontRenderRequest & options, const FontDeviceOptions &devopt, FontRect & outTextRect, FontResources & resources) {

	using SimpleFontShaderDescriptor = Renderer::SimpleFontShaderDescriptor;

	auto &shres = frame->GetResourceManager()->GetShaderResource();
	if (!m_ShaderHandle) {
		shres.Load<SimpleFontShaderDescriptor>(frame->GetControllCommandQueue(), m_ShaderHandle, "Font/Simple");
	}

	auto trt = frame->GetDevice()->AllocateTextureRenderTask();
	if (!trt)
		return false;

	DataClasses::Fonts::Descriptor dummy;
	dummy.Size = options.m_Size;
	auto tsize = TextSize(text.c_str(), &dummy, false);

	trt->SetFrame(frame);
	trt->SetTarget(resources.m_Texture, emath::MathCast<emath::ivec2>(tsize.m_CanvasSize));
	trt->Begin();

	auto &q = trt->GetCommandQueue();

	using namespace ::MoonGlare::Renderer;
	using namespace ::MoonGlare::Renderer::Commands;
	auto key = CommandKey();

	auto shb = shres.GetBuilder<SimpleFontShaderDescriptor>(q, m_ShaderHandle);

	using Uniform = SimpleFontShaderDescriptor::Uniform;
	shb.Bind();
	shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(glm::translate(glm::mat4(), math::vec3(tsize.m_TextPosition, 0))));

	VirtualCamera Camera;
	Camera.SetDefaultOrthogonal(tsize.m_CanvasSize);

	shb.Set<Uniform::CameraMatrix>(Camera.GetProjectionMatrix());

	auto c = options.m_Color;
	shb.Set<Uniform::BackColor>(emath::fvec3(c[0], c[1], c[2]));

	bool fullsucc = GenerateCommands(q, frame, text, options);

	trt->End();

	if (devopt.m_UseUniformMode) {
		float Aspect = (float)devopt.m_DeviceSize[0] / (float)devopt.m_DeviceSize[1];
		auto coeff = math::fvec2(1) / math::fvec2(devopt.m_DeviceSize[0], devopt.m_DeviceSize[1]) * math::fvec2(Aspect * 2.0f, 2.0f);
		tsize.m_CanvasSize = tsize.m_CanvasSize * coeff;
		tsize.m_TextBlockSize = tsize.m_TextBlockSize * coeff;
		tsize.m_TextPosition = tsize.m_TextPosition * coeff;
	}

	outTextRect = tsize;

	auto su = tsize.m_CanvasSize;
	Graphic::QuadArray3 Vertexes{
		Graphic::vec3(0, su[1], 0),
		Graphic::vec3(su[0], su[1], 0),
		Graphic::vec3(su[0], 0, 0),
		Graphic::vec3(0, 0, 0),
	};
	float w1 = 0.0f;
	float h1 = 0.0f;
	float w2 = 1.0f;
	float h2 = 1.0f;
	Graphic::QuadArray2 TexUV{
		Graphic::vec2(w1, h1),
		Graphic::vec2(w2, h1),
		Graphic::vec2(w2, h2),
		Graphic::vec2(w1, h2),
	};

	{
		auto &m = frame->GetMemory();
		using ichannels = Renderer::Configuration::VAO::InputChannels;

		auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, resources.m_VAO, true);
		vaob.BeginDataChange();

		vaob.CreateChannel(ichannels::Vertex);
		vaob.SetChannelData<float, 3>(ichannels::Vertex, (const float*)m.Clone(Vertexes), Vertexes.size());

		vaob.CreateChannel(ichannels::Texture0);
		vaob.SetChannelData<float, 2>(ichannels::Texture0, (const float*)m.Clone(TexUV), TexUV.size());

		vaob.CreateChannel(ichannels::Index);
		static constexpr std::array<uint8_t, 6> IndexTable = { 0, 1, 2, 0, 2, 3, };
		vaob.SetIndex(ichannels::Index, IndexTable);

		vaob.EndDataChange();
		vaob.UnBindVAO();
	}

	frame->Submit(trt);

	return fullsucc;
}

} //namespace Fonts
} //namespace DataClasses
} //namespace MoonGlare 
