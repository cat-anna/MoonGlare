#include <pch.h>
#include <MoonGlare.h>
#include "iFont.h"

namespace DataClasses {
namespace Fonts {

GABI_IMPLEMENT_ABSTRACT_CLASS(Wrapper);

//-----------------------------------------------------------------------------

GABI_IMPLEMENT_CLASS(EmptyWrapper);

void EmptyWrapper::Render(Graphic::cRenderDevice &dev) { }
void EmptyWrapper::RenderMesh(Graphic::cRenderDevice &dev) { }

//-----------------------------------------------------------------------------

GABI_IMPLEMENT_ABSTRACT_CLASS(iFont);

iFont::iFont(const string& Name): 
		BaseClass(Name) {
}

iFont::~iFont() {
}

//-----------------------------------------------------------------------------

bool iFont::DoInitialize() {
	return BaseClass::DoInitialize();
}

bool iFont::DoFinalize() {
	return BaseClass::DoFinalize();
}

} //namespace Fonts
} //namespace DataClasses
