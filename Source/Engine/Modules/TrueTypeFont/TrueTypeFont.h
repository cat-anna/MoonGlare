/*
  * Generated by cppsrc.sh
  * On 2015-05-26 18:55:35,14
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TrueTypeFont_H
#define TrueTypeFont_H

namespace MoonGlare {
namespace Modules {
namespace TrueTypeFont {

using namespace DataClasses::Fonts;

struct FontGlyph {
    bool m_Loaded = false;
    Renderer::MaterialResourceHandle m_GlyphMaterial;


    Graphic::vec2 m_Advance;
    Graphic::vec2 m_Position;
    Graphic::vec2 m_BitmapSize;
    Graphic::vec2 m_TextureSize;//normalized value
};

using UniqueFontGlyph = std::unique_ptr < FontGlyph >;
using FontGlyphMap = std::map < wchar_t, UniqueFontGlyph >;

class TrueTypeFont : public iFont {
    SPACERTTI_DECLARE_STATIC_CLASS(TrueTypeFont, iFont);
public:
    TrueTypeFont(const string& Name);
    virtual ~TrueTypeFont();

    FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const override;

protected:
    bool DoInitialize() override;
    bool DoFinalize() override;

    FontGlyph* GetGlyph(wchar_t codepoint, Renderer::Commands::CommandQueue *q, Renderer::Frame * frame) const;
    virtual bool GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame *frame, const std::wstring &text, const FontRenderRequest &options) override;
private: 
    StarVFS::ByteTable m_FontFile;
    FT_Face m_FontFace = nullptr; 
    float m_CacheHight;

    mutable FontGlyphMap m_GlyphCache;
};

} //namespace TrueTypeFont 
} //namespace Modules 
} //namespace MoonGlare 

#endif