#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H 
#include FT_GLYPH_H

namespace MoonGlare {
namespace Modules {
namespace TrueTypeFont {

extern FT_Library ftlib;

} //namespace TrueTypeFont 
} //namespace Modules 
} //namespace MoonGlare 

#include "FontGlyph.h"
