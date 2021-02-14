#pragma once
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

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	AddLog(Error, "FreeImage [Format: " << FreeImage_GetFormatFromFIF(fif) << "] error: " << message);
}

static void DibDeallocator(void * dib) {
	if (dib != nullptr)
		FreeImage_Unload((FIBITMAP*)dib);
}

#endif
