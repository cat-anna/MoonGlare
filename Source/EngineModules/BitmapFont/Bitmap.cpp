/*
	Generated by cppsrc.sh
	On 2014-12-16  6:13:04,05
	by Paweu
*/

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/DataClasses/iFont.h>
#include "Bitmap.h"

#include "Core/Engine.h"

#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/RenderInput.h>

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/TextureRenderTask.h>
#include <Renderer/Resources/ResourceManager.h>

namespace MoonGlare {
namespace Modules {
namespace BitmapFont {
	
struct BitmapFontModule : public MoonGlare::Modules::ModuleInfo {
	BitmapFontModule(): BaseClass("BitmapFont", ModuleType::Functional) { }
	bool Initialize() override {
		FontClassRegister::Register<BitmapFont> ClassReg;
		return true;
	}
};
DEFINE_MODULE(BitmapFontModule);

//----------------------------------------------------------------

SPACERTTI_IMPLEMENT_STATIC_CLASS(BitmapFont);

BitmapFont::BitmapFont(const string& Name): 
		BaseClass(Name) {
}

BitmapFont::~BitmapFont() {
	Finalize();
}

//----------------------------------------------------------------

bool BitmapFont::DoInitialize(){
	if (!BaseClass::DoInitialize())
		return false;

	auto meta = OpenMetaData();
	if (!meta) {
		//silently ignore
		return false;
	}
	auto root = meta->document_element();

	FileSystem::DirectoryReader reader(DataPath::Fonts);
	auto fpath = reader.translate(root.child("Bitmap").text().as_string(ERROR_STR));

	//m_Material = 

	auto *e = Core::GetEngine();
	auto *rf = e->GetWorld()->GetRendererFacade();
	auto *resmgr = rf->GetResourceManager();

	auto matb = resmgr->GetMaterialManager().GetMaterialBuilder(m_Material, true);
	matb.SetDiffuseColor(emath::fvec4(1));
	matb.SetDiffuseMap("file://" + fpath);

	StarVFS::ByteTable data;
	if (!GetFileSystem()->OpenFile(root.child("BFD").text().as_string(ERROR_STR), DataPath::Fonts, data)) {
		AddLog(Error, "Unable to open the bfd file for font '" << GetName() << "'");
		return false;
	}
	memcpy(reinterpret_cast<char*>(&m_BFD), data.get(), sizeof(cBFDHeader));

	return true;
}

//-----------------------------------------------------------------------------

BitmapFont::FontRect BitmapFont::TextSize(const wstring & text, const Descriptor * style, bool UniformPosition) const {

	float h = static_cast<float>(m_BFD.CharWidth);
	if (style && style->Size > 0) 
		h = style->Size;
	float w_mult = h / static_cast<float>(m_BFD.CharWidth);
	unsigned fx = m_BFD.Width / m_BFD.CharWidth;
	float x = 0.0f;

	auto cstr = text.c_str();
	while (*cstr) {
		auto wc = *cstr;
		++cstr;
		char c = static_cast<char>(wc);

		unsigned kid = static_cast<unsigned>(c) - m_BFD.BeginingKey;
		if (kid > 255) 
			kid = fx;

		x += (m_BFD.KeyWidths[kid] + 1) * w_mult;
	}

	FontRect rect;
	rect.m_CanvasSize = math::vec2(x, h);
	rect.m_TextBlockSize = rect.m_CanvasSize;
	rect.m_TextPosition = math::vec2(0, 0);
	return rect;
}

//-----------------------------------------------------------------------------

bool BitmapFont::GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame *frame, const std::wstring &text, const FontRenderRequest &foptions) {
	if (text.empty())
		return true;

	static const std::array<uint8_t, 6> BaseIndex{ 0, 1, 2, 0, 2, 3, };

	unsigned textlen = text.length();
	unsigned VerticlesCount = textlen * 4;
	unsigned IndexesCount = textlen * BaseIndex.size();

	emath::fvec3 *Verticles = frame->GetMemory().Allocate<emath::fvec3>(VerticlesCount);
	emath::fvec2 *TextureUV = frame->GetMemory().Allocate<emath::fvec2>(VerticlesCount);
	uint16_t *VerticleIndexes = frame->GetMemory().Allocate<uint16_t>(IndexesCount);

	float y = 0/*, z = Pos.z*/;
	float h = static_cast<float>(m_BFD.CharWidth), w;
	if (foptions.m_Size > 0)
		h = foptions.m_Size;
	w = h;
	float w_mult = w / static_cast<float>(m_BFD.CharWidth);
	unsigned fx = m_BFD.Width / m_BFD.CharWidth;
	float x = 0;
	float Cx = m_BFD.Width / static_cast<float>(m_BFD.CharWidth);
	float Cy = m_BFD.Height / static_cast<float>(m_BFD.CharHeight);
	float dw = 1 / Cx;
	float dh = 1 / Cy;

	auto CurrentVertexQuad = Verticles;
	auto CurrentTextureUV = TextureUV;
	auto CurrentIndex = VerticleIndexes;

	auto cstr = text.c_str();
	while (*cstr) {
		auto wc = *cstr;
		++cstr;

		char c = static_cast<char>(wc);

		unsigned kid = static_cast<unsigned>(c) - m_BFD.BeginingKey;
		if (kid > 255) kid = fx;
		unsigned kol = kid % fx;
		unsigned line = kid / fx;
		float u = kol / Cx;
		float v = 1.0f - (line / Cy);

		CurrentVertexQuad[0] = emath::fvec3(x + w,	y,		0);
		CurrentVertexQuad[1] = emath::fvec3(x,		y,		0);
		CurrentVertexQuad[2] = emath::fvec3(x,		y + h,	0);
		CurrentVertexQuad[3] = emath::fvec3(x + w,	y + h,	0);

		CurrentTextureUV[0] = emath::fvec2(u + dw,	v);
		CurrentTextureUV[1] = emath::fvec2(u,		v);
		CurrentTextureUV[2] = emath::fvec2(u,		v - dh);
		CurrentTextureUV[3] = emath::fvec2(u + dw,	v - dh);

		size_t basevertex = CurrentVertexQuad - Verticles;
		for (auto idx : BaseIndex) {
			*CurrentIndex = idx + basevertex;
			++CurrentIndex;
		}

		x += (m_BFD.KeyWidths[kid] + 1) * w_mult;

		CurrentVertexQuad += 4;
		CurrentTextureUV += 4;
	}

	Renderer::VAOResourceHandle vao{ 0 };
	{
		if (!frame->AllocateFrameResource(vao))
			return false;

		auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, vao, false);
		vaob.BeginDataChange();

		using ichannels = Renderer::Configuration::VAO::InputChannels;
		vaob.CreateChannel(ichannels::Vertex);
		vaob.SetChannelData<float, 3>(ichannels::Vertex, &Verticles[0][0], VerticlesCount);

		vaob.CreateChannel(ichannels::Texture0);
		vaob.SetChannelData<float, 2>(ichannels::Texture0, &TextureUV[0][0], VerticlesCount);

		vaob.CreateChannel(ichannels::Index);
		vaob.SetIndex(ichannels::Index, VerticleIndexes, IndexesCount);

		vaob.EndDataChange();
	}
	using namespace ::MoonGlare::Renderer;
	auto key = Commands::CommandKey();

	auto *e = Core::GetEngine();
	auto *rf = e->GetWorld()->GetRendererFacade();
	auto *resmgr = rf->GetResourceManager();

	auto *mat = resmgr->GetMaterialManager().GetMaterial(m_Material);

	auto texarg = q.PushCommand<Renderer::Commands::Texture2DResourceBind>(key);
	texarg->m_Handle = mat->m_DiffuseMap;
	texarg->m_HandleArray = resmgr->GetTextureResource().GetHandleArrayBase();
		
	auto arg = q.PushCommand<Renderer::Commands::VAODrawTriangles>(key);
	arg->m_NumIndices = IndexesCount;
	arg->m_IndexValueType = Renderer::Device::TypeInfo<std::remove_reference_t<decltype(*VerticleIndexes)>>::TypeId;

	return true;
}

} //namespace BitmapFont 
} //namespace Modules 
} //namespace MoonGlare 
