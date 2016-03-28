#ifndef FREEIMAGEUTILS_H
#define FREEIMAGEUTILS_H
#ifndef _DISABLE_FREEIMAGE_LIB_

#include <FreeImage.h>

void RGBToBGR(FIBITMAP *dib){
	BYTE *bits = FreeImage_GetBits(dib);
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	for(int i = 0, j = height * width; i < j; ++i){
		BYTE b = bits[0];
		bits[0] = bits[2];
		bits[2] = b;
		bits += 3;
	}
}

void RGBAToBGRA(FIBITMAP *dib){
	BYTE *bits = FreeImage_GetBits(dib);
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	for(int i = 0, j = height * width; i < j; ++i){
		BYTE b = bits[0];
		bits[0] = bits[2];
		bits[2] = b;
		bits += 4;
	}
}

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	AddLog(Error, "FreeImage [Format: " << FreeImage_GetFormatFromFIF(fif) << "] error: " << message);
}

#endif
#endif
