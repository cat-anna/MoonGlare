#pragma once
#ifndef DISABLE_FREEIMAGE_LIB

#include <FreeImage.h>
#include <fmt/format.h>
#include <orbit_logger.h>

static void SwapRedAndBlue(FIBITMAP *dib) {
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
    AddLog(Error, fmt::format("FreeImage [Format: {}] error: {}", FreeImage_GetFormatFromFIF(fif),
                              message));
}

static void DibDeallocator(FIBITMAP *dib) {
    if (dib != nullptr) {
        FreeImage_Unload((FIBITMAP *)dib);
    }
}

#endif
