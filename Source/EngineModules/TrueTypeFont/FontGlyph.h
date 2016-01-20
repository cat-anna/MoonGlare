#pragma once

namespace MoonGlare {
namespace Modules {
namespace TrueTypeFont {

struct FontGlyph {
	//Graphic::VAO m_VAO;
	Graphic::Texture m_Texture;
	Graphic::vec2 m_Advance;
	Graphic::vec2 m_Position;
	Graphic::vec2 m_BitmapSize;
	Graphic::vec2 m_TextureSize;//normalized value
	bool m_Ready = false; //glyph is prepared to be rendered
};

using UniqueFontGlyph = std::unique_ptr < FontGlyph > ;
using FontGlyphMap = std::map < wchar_t, UniqueFontGlyph > ;

} //namespace TrueTypeFont 
} //namespace Modules 
} //namespace MoonGlare 