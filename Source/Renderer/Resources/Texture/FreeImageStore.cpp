#include "pch.h"

#include "FreeImageStore.h"
#include "FreeImageUtils.h"

namespace MoonGlare::Renderer::Resources::Texture {

std::string FormatDateTime() {
    time_t rawtime;
    struct tm  timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = *localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d_%m_%Y_%H_%M_%S", &timeinfo);
    std::string str(buffer);

    return str;
}

void FreeImageStore::Do(ResourceLoadStorage &storage) {
    if (fname.empty()) {        fname = fmt::format("screenshoot_{}.png", FormatDateTime());    }
    AddLogf(Info, "Starting image store: %s", fname.c_str());

    auto bitmap = FreeImage_ConvertFromRawBits(memory.get(), size[0], size[1], size[0] * (bpp / 8), bpp,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
    FreeImage_Save(FIF_PNG, bitmap, fname.c_str());    FreeImage_Unload(bitmap);

    AddLogf(Info, "Stored image %s", fname.c_str());
}

}
