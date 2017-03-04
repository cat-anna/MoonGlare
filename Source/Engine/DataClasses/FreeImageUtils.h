#ifndef FREEIMAGEUTILS_H
#define FREEIMAGEUTILS_H
#ifndef _DISABLE_FREEIMAGE_LIB_

#include <FreeImage.h>

static void SwapRedAndBlue(FIBITMAP *dib){
	unsigned height = FreeImage_GetHeight(dib);

	unsigned linebytes = FreeImage_GetLine(dib);
	unsigned bpp = FreeImage_GetBPP(dib) / 8;

	for (unsigned h = 0; h < height; ++h) {
		BYTE *line = FreeImage_GetScanLine(dib, h);
		for (unsigned w = 0; w < linebytes; w += bpp) {
			std::swap(line[w + FI_RGBA_BLUE], line[w + FI_RGBA_RED]);
		}
	}
}

#endif
#endif
