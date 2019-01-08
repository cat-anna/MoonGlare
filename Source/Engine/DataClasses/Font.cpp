#include <pch.h>

#define NEED_VAO_BUILDER
#define NEED_MATERIAL_BUILDER

#include <nfMoonGlare.h>
#include "Font.h"

#include "Core/Engine.h"

#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>

#include <Renderer/Frame.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/TextureRenderTask.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/Shader/ShaderResource.h>
#include <Renderer/Resources/Mesh/VAOResource.h>

#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/TextureRenderTask.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Device/Types.h>
#include <Renderer/Resources/Shader/ShaderResource.h>
#include <Renderer/Resources/Mesh/VAOResource.h>
#include <Renderer/Resources/Texture/TextureResource.h>
#include <Renderer/Resources/MaterialManager.h>

#include <Renderer/VirtualCamera.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H 
#include FT_GLYPH_H

#ifdef DEBUG_DUMP
#include <FreeImage.h>
#endif

namespace MoonGlare {
namespace DataClasses {

FT_Library ftlib = nullptr;

struct TrueTypeFontModule {
    TrueTypeFontModule() {
        auto error = FT_Init_FreeType(&ftlib);
        if (error) {
            AddLogf(Error, "Unable to initialize FreeType library. Code: %d", error);
            return;
        }

        FT_Int major, minor, patch;
        FT_Library_Version(ftlib, &major, &minor, &patch);

        AddLogf(System, "FreeType version: %d.%d.%d", major, minor, patch);
    }

   ~TrueTypeFontModule(){
        auto error = FT_Done_Library(ftlib);
        if (error) {
            AddLogf(Error, "Unable to finalize FreeType library. Code: %d", error);
        }
    }
};

//----------------------------------------------------------------

iFont::iFont(const string& uri) : fileUri(uri) {
    static TrueTypeFontModule mod;
}

iFont::~iFont() {}

//----------------------------------------------------------------

void iFont::DumpFontCodePoints() {
    FT_ULong  charcode;
    FT_UInt   gindex;
    size_t cnt = 0;

    charcode = FT_Get_First_Char(m_FontFace, &gindex);
    std::stringstream ss;
    while (gindex != 0) {
        ++cnt;
        if (charcode < 256 && isprint(charcode))
            ss << fmt::format("{}[0x{:02x}] ", (char)charcode, (int)charcode);
        else
            ss << fmt::format("\\{}[0x{:02x}] ", (int)charcode, (int)charcode);
        if ((cnt % 16) == 0)
            ss << "\n";
        charcode = FT_Get_Next_Char(m_FontFace, charcode, &gindex);
    }
    auto str = ss.str();
    AddLog(Debug, fmt::format("TTF[{}]: Available code points:[{}]\n{}", cnt, fileUri.c_str(), str));
}

void iFont::DumpFacesTexture() {
#ifdef DEBUG_DUMP
    uint32_t texSize = faceTextureSize * FontFacesPerDim;
    FIBITMAP *bitmap = FreeImage_Allocate(texSize, texSize, 8);    uint8_t *out = FreeImage_GetBits(bitmap);    for (size_t pos = 0; pos < texSize*texSize; ++pos) {        out[pos] = facesTexture[pos] & 0xFF;    }    std::string fname = "logs/" + fileUri.substr(fileUri.rfind('/')) + ".png";    FreeImage_Save(FIF_PNG, bitmap, fname.c_str());    FreeImage_Unload(bitmap);
#endif
}

//----------------------------------------------------------------

bool iFont::Initialize() {
    if (IsReady())
        return true;          
    SetReady(true);

    if (!GetFileSystem()->OpenFile(fileUri, DataPath::URI, fontFileMemory)) {
        AddLog(Error, "Unable to open font file!");
        return false;
    }

    auto error = FT_New_Memory_Face(ftlib,
        (const FT_Byte*)fontFileMemory.get(),	        /* first byte in memory */
        fontFileMemory.size(),							/* size in bytes        */
        0,										    	/* face_index           */
        &m_FontFace);

    if (error) {
        AddLogf(Error, "Unable to load font: '%s' error code: %d", fileUri.c_str(), error);
        return false;
    }

#if 0 
    //DEBUG_DUMP
    DumpFontCodePoints();
#endif

    faceTextureSize = 64;
    m_CacheHight = 40;// ((float)faceTextureSize) * 0.75;
    uint32_t texSize = faceTextureSize * FontFacesPerDim;
    uint32_t texBytes = texSize * texSize;
    facesTexture.reset(new uint16_t[texBytes]);
    memset(facesTexture.get(), 0, texBytes);

    error = FT_Set_Char_Size(
        m_FontFace,			                /* handle to face object           */
        0,					                /* char_width in 1/64th of points  */
        (FT_F26Dot6(m_CacheHight)*64.0f),	    /* char_height in 1/64th of points */
        96,			    	                /* horizontal device resolution    */
        96);				                /* vertical device resolution      */

    if (error) {
        AddLogf(Error, "Unable to set font size: '%s' error code: %d", fileUri.c_str(), error);
        return Finalize();
    }

    //todo: select unicode charmap

    auto *e = Core::GetEngine();
    auto *rf = e->GetWorld()->GetRendererFacade();

    static constexpr wchar_t DefaultChars[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,!@#$%^&*-_=+?()[]{};:'<>\\/\"0123456789";
    for (auto c : DefaultChars)
        GetGlyph(c);

    rf->GetAsyncLoader()->PostTask([this, rf](Renderer::ResourceLoadStorage &storage) {
        auto *resmgr = rf->GetResourceManager();
        auto &texR = resmgr->GetTextureResource();

        using namespace Renderer::Device;

        Renderer::MaterialTemplate matT;
        matT.diffuseMap.enabled = true;
        matT.diffuseMap.textureHandle = texR.CreateTexture(storage.m_Queue, facesTexture.get(), { 1,1 }, {}, PixelFormat::RedGreen, PixelFormat::RedGreen, ValueFormat::UnsignedByte);
        facesMaterial = resmgr->GetMaterialManager().CreateMaterial("", matT);
    });

    AddLogf(Debug, "Initialized resource '%s' of class '%s'", "!", typeid(*this).name());
    return true;
}

bool iFont::Finalize() {
    if (!IsReady())
        return true;           
    SetReady(false);

    facesTexture.reset();
    glyphCache.clear();
    FT_Done_Face(m_FontFace);
    m_FontFace = nullptr;
    fontFileMemory.reset();
    return true;
}

//----------------------------------------------------------------

bool iFont::RenderText(const std::wstring & text, Renderer::Frame * frame, const FontRenderRequest & options, 
        const FontDeviceOptions &devopt, FontRect & outTextRect, FontResources & resources) {
    using PassthroughShaderDescriptor = Renderer::PassthroughShaderDescriptor;

    auto &shres = frame->GetResourceManager()->GetShaderResource();
    if (!m_ShaderHandle) {
        shres.Load(m_ShaderHandle, "Passthrough");
    }

    auto trt = frame->GetDevice()->AllocateTextureRenderTask();
    if (!trt)
        return false;

    DataClasses::Descriptor dummy;
    dummy.Size = options.m_Size;
    auto tsize = TextSize(text.c_str(), &dummy, false);

    trt->SetFrame(frame);
    trt->SetTarget(resources.m_Texture, emath::MathCast<emath::ivec2>(tsize.m_CanvasSize));
    trt->Begin();

    auto &q = trt->GetCommandQueue();

    using namespace ::MoonGlare::Renderer;
    using namespace ::MoonGlare::Renderer::Commands;
    auto key = CommandKey();

    auto shb = shres.GetBuilder(q, m_ShaderHandle);

    using Uniform = PassthroughShaderDescriptor::Uniform;
    shb.Bind();
    shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(glm::translate(glm::identity<glm::fmat4>(), math::vec3(tsize.m_TextPosition, 0))));

    VirtualCamera Camera;
    Camera.SetDefaultOrthogonal(tsize.m_CanvasSize);

    shb.Set<Uniform::CameraMatrix>(Camera.GetProjectionMatrix());
    shb.Set<Uniform::BackColor>(options.m_Color);

    GenerateCommands(q, frame, text, options);

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
    math::vec3 Vertexes[4] = {
        math::vec3(0, su[1], 0),
        math::vec3(su[0], su[1], 0),
        math::vec3(su[0], 0, 0),
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

    {
        auto &m = frame->GetMemory();
        using ichannels = Renderer::Configuration::VAO::InputChannels;

        auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, resources.m_VAO, true);
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

    frame->Submit(trt);

    return true;
}

//----------------------------------------------------------------

iFont::FontRect iFont::TextSize(const wstring & text, const Descriptor * style, bool UniformPosition) const {

    float topline = 0;
    float bottomline = 0;
    float width = 0;

    float h;
    if (style) {
        h = style->Size;
    } else {
        h = m_CacheHight;
    }

    const float char_scale = h / faceTextureSize;
    const wstring::value_type *cstr = text.c_str();

    while (*cstr) {
        wchar_t c = *cstr;
        ++cstr;              

        auto *g = GetGlyph(c);

        emath::fvec2 BitmapSize = g->charSize * char_scale;
        emath::fvec2 CharPos = g->position * char_scale;

        topline = std::min(topline, CharPos.y());
        bottomline = std::max(bottomline, CharPos.y() + BitmapSize.y());

        //		auto subpos = pos + chpos;
        //		float bsy = bs.y;
        //
        //		if (UniformPosition) {
        //			subpos = subpos / ScreenSize * math::fvec2(Aspect * 2.0f, 2.0f);
        //			bs = bs / ScreenSize * math::fvec2(Aspect * 2.0f, 2.0f);
        //		}
        //
        width += g->advance.x() * char_scale;
        //		hmax = math::max(hmax, bsy);
    }
    //	pos.y = hmax;
    //
    //	if (UniformPosition)
    //		wrapper->m_size = pos / math::fvec2(ScreenSize) * math::fvec2(Aspect * 2.0f, 2.0f);
    //	else
    //		wrapper->m_size = pos;

    FontRect rect;
    rect.m_CanvasSize = math::fvec2(width, -topline + bottomline);
    rect.m_TextPosition = math::fvec2(0, -topline);
    rect.m_TextBlockSize = math::fvec2(width, h);
    return rect;
}

//
//  _                    _ 	___________________
// | |                  | |					   |topline
// | |_  __  __   __ _  | |	___________________|_______________	baseline
// | __| \ \/ /  / _` | | |				  
// | |_   >  <  | (_| | | |				
//  \__| /_/\_\  \__, | |_|	____________________
//                __/ |    					   |
//               |___/     	___________________|bottomline
//
// |-----------------------| width

bool iFont::GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame * frame, const std::wstring &text, const FontRenderRequest & options) {
    if (text.empty())
        return true;


    Renderer::VAOResourceHandle vao{ };
    if (!frame->AllocateFrameResource(vao))
        return false;

    unsigned textlen = text.length();
    unsigned VerticlesCount = textlen * 4;
    unsigned IndexesCount = textlen * 6;
    emath::fvec3 *Verticles = frame->GetMemory().Allocate<emath::fvec3>(VerticlesCount);
    emath::fvec2 *TextureUV = frame->GetMemory().Allocate<emath::fvec2>(VerticlesCount);
    uint16_t *VerticleIndexes = frame->GetMemory().Allocate<uint16_t>(IndexesCount);

    {
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
        vaob.BindVAO();
    }

    float h = m_CacheHight;

    if (options.m_Size > 0)
        h = options.m_Size;

    for(auto c : text)
        GetGlyph(c);

    if (faceTextureDirty) {
        faceTextureDirty = false;
        ReloadFacesTexture(frame, q);
#ifdef DEBUG_DUMP
        DumpFacesTexture();
#endif
    }

    const float char_scale = h / faceTextureSize;
    const wstring::value_type *cstr = text.c_str();
    emath::fvec2 pos(0,0);
    float hmax = h;

    auto CurrentVertexQuad = Verticles;
    auto CurrentTextureUV = TextureUV;
    auto CurrentIndex = VerticleIndexes;
    
    auto *resmgr = frame->GetResourceManager();

    auto texbind = q.PushCommand<Renderer::Commands::Texture2DResourceBind>();
    auto mat = resmgr->GetMaterialManager().GetMaterial(facesMaterial);
    texbind->m_HandlePtr = resmgr->GetTextureResource().GetHandleArrayBase() + mat->mapTexture[Renderer::Material::MapType::Diffuse].index;
  
    emath::fvec2 bs = { faceTextureSize, faceTextureSize };
    bs *= char_scale;

    while (*cstr) {
        wchar_t c = *cstr;
        ++cstr;

        auto *g = GetGlyph(c);
        
        auto chpos = g->position;
        chpos *= char_scale;
        auto subpos = pos + chpos;

        if (c != L' ') {
            CurrentVertexQuad[0] = emath::fvec3(subpos.x() + 0, subpos.y() + bs.y(), 0);
            CurrentVertexQuad[1] = emath::fvec3(subpos.x() + 0, subpos.y() + 0, 0);
            CurrentVertexQuad[2] = emath::fvec3(subpos.x() + bs.x(), subpos.y() + 0, 0);
            CurrentVertexQuad[3] = emath::fvec3(subpos.x() + bs.x(), subpos.y() + bs.y(), 0);

            emath::fvec2 uvBase = g->fontFacePosition / FontFacesPerDim;
            CurrentTextureUV[0] = emath::fvec2(uvBase[0],               uvBase[1]);
            CurrentTextureUV[1] = emath::fvec2(uvBase[0],               uvBase[1] + DeltaTexUV);
            CurrentTextureUV[2] = emath::fvec2(uvBase[0] + DeltaTexUV,  uvBase[1] + DeltaTexUV);
            CurrentTextureUV[3] = emath::fvec2(uvBase[0] + DeltaTexUV,  uvBase[1]);

            size_t basevertex = CurrentVertexQuad - Verticles;
            size_t baseIndex = CurrentIndex - VerticleIndexes;

            static constexpr unsigned BaseIndex[] = { 0, 1, 2, 0, 2, 3, };
            for (auto idx : BaseIndex) {
                *CurrentIndex = static_cast<uint16_t>(idx + basevertex);
                ++CurrentIndex;
            }

            auto arg = q.PushCommand<Renderer::Commands::VAODrawTrianglesBase>();
            arg->m_NumIndices = 6;
            arg->m_BaseIndex = baseIndex * 2;
            arg->m_IndexValueType = Renderer::Device::TypeInfo<std::remove_reference_t<decltype(*VerticleIndexes)>>::TypeId;

            CurrentVertexQuad += 4;
            CurrentTextureUV += 4;
        }

        pos.x() += g->advance.x() * char_scale;
        hmax = math::max(hmax, bs.y());
    }

    return true;
}

void iFont::ReloadFacesTexture(MoonGlare::Renderer::Frame * frame, MoonGlare::Renderer::Commands::CommandQueue & q) {
    auto *resmgr = frame->GetResourceManager();
    auto &texR = resmgr->GetTextureResource();

    Renderer::Configuration::TextureLoad tload = Renderer::Configuration::TextureLoad::Default();
    tload.m_Filtering = Renderer::Configuration::Texture::Filtering::Linear;
    tload.m_Edges = Renderer::Configuration::Texture::Edges::Repeat;
    using ChannelSwizzle = Renderer::Configuration::Texture::ChannelSwizzle;
    tload.m_Flags.generateMipMaps = false;
    tload.m_Swizzle = ChannelSwizzle::R;
    tload.m_Swizzle.A = ChannelSwizzle::G;
    tload.m_Swizzle.enable = true;

    uint32_t texSize = faceTextureSize * FontFacesPerDim;

    using namespace Renderer::Device;

    texR.SetTexturePixels(facesMaterial.deviceHandle->mapTexture[Renderer::Material::MapType::Diffuse], q,
        facesTexture.get(), { texSize,texSize }, tload, PixelFormat::RedGreen, PixelFormat::RedGreen,
        false, ValueFormat::UnsignedByte);
}

iFont::FontGlyph* iFont::GetGlyph(wchar_t codepoint) const {
    auto it = glyphCache.find(codepoint);
    if (it != glyphCache.end())
        return &it->second;
    
    auto *glyph = &glyphCache[codepoint];

    auto glyph_index = FT_Get_Char_Index(m_FontFace, codepoint);
    auto load_flags = FT_LOAD_DEFAULT;// FT_LOAD_RENDER;
    auto error = FT_Load_Glyph(
        m_FontFace,    /* handle to face object */
        glyph_index,   /* glyph index           */
        load_flags);   /* load flags, see below */

    if (error) {
        AddLogf(Error, "Unable to load glyph for char: 0x%x", codepoint);
        return nullptr;
    }

    FT_Glyph ffglyph = nullptr;
    if (FT_Get_Glyph(m_FontFace->glyph, &ffglyph)) {
        AddLogf(Error, "Get glyph failed! codepoint: 0x%x", (unsigned)codepoint);
        return nullptr;
    }

    // Convert The Glyph To A Bitmap.
    if (FT_Glyph_To_Bitmap(&ffglyph, FT_RENDER_MODE_NORMAL, 0, 1)) {
        AddLogf(Error, "Glyph  render failed! codepoint: 0x%x", (unsigned)codepoint);
        FT_Done_Glyph(ffglyph);
        return nullptr;
    }

    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)ffglyph;
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    unsigned pos = (faceTextureSize - bitmap.rows);// / 2;
    glyph->position = emath::fvec2(bitmap_glyph->left, (faceTextureSize - bitmap_glyph->top));
    glyph->advance = emath::fvec2(m_FontFace->glyph->advance.x / 64.0f, 0);
    glyph->charSize = emath::fvec2(bitmap.width + 1, bitmap.rows + 1);      
    glyph->fontFacePosition = { 0, 0, };
    glyph->faceIndex = 0;

    if (bitmap.rows > faceTextureSize || bitmap.width > faceTextureSize) {
        //TODO: report font error!
    }
    else
    if (bitmap.rows > 0 && bitmap.width > 0) {
        auto idx = faceAllocIndex++;
        faceTextureDirty = true;

        glyph->fontFacePosition = { idx % FontFacesPerDim , idx / FontFacesPerDim };

        for (unsigned j = 0; j < bitmap.rows; j++) {
            unsigned l = faceTextureSize - j - 1;// -pos - 1;
            auto line = GetTextureScanLine(glyph->fontFacePosition, l);
            for (unsigned i = 0; i < bitmap.width; i++) {
                uint8_t value = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j];
                uint8_t alpha = value != 0 ? 0xFF : 0;
                line[i] = value | alpha << 8;
            }
        }
    } else {
    }

    auto faceglyph = m_FontFace->glyph;
    DebugLogf(Debug, "TTF[%s]: char %c[0x%x] size:(%3d;%3d) bitmap:(%3d;%3d) pos:(%4d;%4d) bsize(%3d;%3d)",
        fileUri.c_str(),
        (unsigned)codepoint, (unsigned)codepoint,
        (int)faceglyph->advance.x, (int)faceglyph->advance.y,
        (int)bitmap.width, (int)bitmap.rows,
        (int)bitmap_glyph->left, (int)bitmap_glyph->top,
        (int)bitmap.width, (int)bitmap.rows
    );                     

    FT_Done_Glyph(ffglyph);
    return glyph;
}

} //namespace DataClasses
} //namespace MoonGlare 
