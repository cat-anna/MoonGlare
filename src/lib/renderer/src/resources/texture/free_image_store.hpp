#if 0

#pragma once

#include "../../iAsyncLoader.h"
#include "../../nfRenderer.h"
#include <math/EigenMath.h>
#include <memory>
#include <string>

struct FIBITMAP;
enum FREE_IMAGE_FORMAT;

namespace MoonGlare::Renderer::Resources::Texture {

class FreeImageStore : public iAsyncTask {
public:
    FreeImageStore(std::unique_ptr<uint8_t[]> memory, emath::ivec2 size, unsigned bpp, std::string fname = "")
        : memory(std::move(memory)), size(size), bpp(bpp), fname(std::move(fname)) {}

    void Do(ResourceLoadStorage &storage) override;

private:
    std::unique_ptr<uint8_t[]> memory;
    emath::ivec2 size;
    std::string fname;
    unsigned bpp;
};

} // namespace MoonGlare::Renderer::Resources::Texture
#endif