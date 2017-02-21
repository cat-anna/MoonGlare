#include <pch.h>
#include <MoonGlare.h>
#include "iFont.h"

namespace MoonGlare {
namespace DataClasses {
namespace Fonts {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(Wrapper);

//-----------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS(EmptyWrapper);

void EmptyWrapper::Render(Graphic::cRenderDevice &dev) { }
void EmptyWrapper::RenderMesh(Graphic::cRenderDevice &dev) { }

//-----------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(iFont);

iFont::iFont(const string& Name): 
		BaseClass(Name) {
}

} //namespace Fonts
} //namespace DataClasses
} //namespace MoonGlare 
