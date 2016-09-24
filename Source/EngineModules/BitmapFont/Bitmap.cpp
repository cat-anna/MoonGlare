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

//#include <Renderer/Commands/ControllCommands.h>
//#include <Renderer/Commands/ShaderCommands.h>
#include <Renderer/Commands/TextureCommands.h>
#include <Renderer/Commands/ArrayCommands.h>
#include <Renderer/RenderInput.h>

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

	if (!GetFileSystem()->OpenTexture(m_Texture, root.child("Bitmap").text().as_string(ERROR_STR), DataPath::Fonts, false)) {
		AddLog(Error, "Unable to load texture for bitmap font: " << m_TextureFile);
		return false;
	}

	auto &texref = m_Texture;
	Graphic::GetRenderDevice()->RequestContextManip([&texref] () {
		texref->SetNearestFiltering();
	});

	StarVFS::ByteTable data;
	if (!GetFileSystem()->OpenFile(root.child("BFD").text().as_string(ERROR_STR), DataPath::Fonts, data)) {
		AddLog(Error, "Unable to open the bfd file for font '" << GetName() << "'");
		return false;
	}
	memcpy(reinterpret_cast<char*>(&m_BFD), data.get(), sizeof(cBFDHeader));

	return true;
}

bool BitmapFont::DoFinalize(){
	return BaseClass::DoFinalize();
}

//----------------------------------------------------------------

FontInstance BitmapFont::GenerateInstance(const wstring &text, const Descriptor *style, bool UniformPosition) const {
	if (text.empty() || !IsReady()) {
		return FontInstance(new EmptyWrapper());
	}

	int textlen = text.length();
	std::vector<math::fvec3> Coords;
	std::vector<math::fvec2> UVs;
	std::vector<unsigned> Index;
	Coords.reserve(textlen * 4);
	UVs.reserve(textlen * 4);
	Index.reserve(textlen * 6);

	Graphic::VAO::MeshData mesh;

	auto ScreenSize = math::fvec2(Graphic::GetRenderDevice()->GetContextSize());
	float Aspect = ScreenSize[0] / ScreenSize[1];

	float y = 0/*, z = Pos.z*/;
	float h = static_cast<float>(m_BFD.CharWidth), w;
	if (style && style->Size > 0) h = style->Size;
	w = h;
	float w_mult = w / static_cast<float>(m_BFD.CharWidth);
	unsigned fx = m_BFD.Width / m_BFD.CharWidth; 
	float x = 0;
	float Cx = m_BFD.Width / static_cast<float>(m_BFD.CharWidth);
	float Cy = m_BFD.Height / static_cast<float>(m_BFD.CharHeight);
	float dw = 1 / Cx;
	float dh = 1 / Cy;

	Graphic::IndexVector BaseIndex{ 0, 1, 2, 0, 2, 3, };

	auto cstr = text.c_str();
	while (*cstr) {
		auto wc = *cstr;
		++cstr;

		if (wc == L' ' && !*cstr) 
			break; //ignore trailing space char

		char c = static_cast<char>(wc);

		unsigned kid = static_cast<unsigned>(c) - m_BFD.BeginingKey;
		if (kid > 255) kid = fx;
		unsigned kol = kid % fx;
		unsigned line = kid / fx;
		float u = kol / Cx;
		float v = 1.0f - (line / Cy);

		math::fvec3 vertex[] = {
			math::fvec3(x + w,	y,		0),
			math::fvec3(x,		y,		0),
			math::fvec3(x,		y + h,	0),
			math::fvec3(x + w,	y + h,	0),
		};

		math::fvec2 uv[] = {
			math::fvec2(u + dw,	v),
			math::fvec2(u,		v),
			math::fvec2(u,		v - dh),
			math::fvec2(u + dw,	v - dh),
		};

		size_t basevertex = Coords.size();
		for (int i = 0; i < 4; ++i){
			if (UniformPosition) {
				Coords.push_back(vertex[i] / math::fvec3(ScreenSize, 1.0f) * math::fvec3(Aspect * 2.0f, 2.0f, 0.0f));
			} else
				Coords.push_back(vertex[i]);
			UVs.push_back(uv[i]);
		//	Index.push_back(Index.size());
		}

		for(auto idx: BaseIndex) 
			Index.push_back(idx + basevertex);

		x += (m_BFD.KeyWidths[kid] + 1) * w_mult;
	}

	auto wr = new BitmapFontWrapper(this);
	wr->m_size = math::vec2(x, h);
	mesh.ElementMode = Graphic::Flags::fTriangles;
	mesh.NumIndices = Index.size();
	wr->m_Mesh = mesh;

	Graphic::NormalVector Normals;
	wr->m_VAO.DelayInit(Coords, UVs, Normals, Index);

	if (style) {
		wr->m_Color = style->Color;
	} else {
		wr->m_Color = Graphic::vec3(1);
	}

	return FontInstance(wr);
}

//----------------BitmapWrapper-------------------------------------------------------------

SPACERTTI_IMPLEMENT_STATIC_CLASS(BitmapFontWrapper);

BitmapFontWrapper::BitmapFontWrapper(const BitmapFont *font): 
		BaseClass(),
		m_Texture(font->GetTexture()) {
	m_AllowSubPixels = false;
}

BitmapFontWrapper::~BitmapFontWrapper() {
	m_VAO.Finalize();
}
 
void BitmapFontWrapper::Render(Graphic::cRenderDevice &dev) {
	dev.CurrentShader()->SetBackColor(m_Color);
	m_Texture->Bind();
	m_VAO.Bind();
	m_VAO.DrawElements(m_Mesh);
}

void BitmapFontWrapper::RenderMesh(Graphic::cRenderDevice &dev) {
	m_VAO.Bind();
	m_VAO.DrawElements(m_Mesh);
}

void BitmapFontWrapper::GenerateCommands(Renderer::CommandQueue & Queue, uint16_t key) {
	Renderer::RendererConf::CommandKey qkey{ key };

	Queue.PushCommand<Renderer::Commands::Texture2DBind>(qkey)->m_Texture = m_Texture->Handle();
	Queue.PushCommand<Renderer::Commands::VAOBind>(qkey)->m_VAO = m_VAO.Handle();

	auto arg = Queue.PushCommand<Renderer::Commands::VAODrawTriangles>(qkey);
	arg->m_NumIndices = m_Mesh.NumIndices;
	arg->m_IndexValueType = GL_UNSIGNED_INT;
}

//----------------BitmapFont::cBFDHeader-------------------------------------------------------------

BitmapFont::cBFDHeader::cBFDHeader(){
	Width = Height = CharWidth = CharHeight = 0;
	BeginingKey = 0;
	memset(KeyWidths, 0, 256);
}

} //namespace BitmapFont 
} //namespace Modules 
} //namespace MoonGlare 
