#pragma once

#include <Renderer/Device/Types.h>

namespace MoonGlare::Renderer {

class TextureLoader {
public:
    virtual ~TextureLoader() {}

    using ImageUniquePtr = std::unique_ptr<void, void(*)(void*)>;

    struct TexturePixelData {
        void* m_Pixels;
        unsigned m_PixelsByteSize;
        Device::ValueFormat m_PixelType;
        Device::PixelFormat m_PixelFormat;
        emath::usvec2 m_PixelSize;
        ImageUniquePtr m_ImageMemory{ nullptr, [](void*) {} };
    };

    virtual bool LoadTexture(const std::string &fpath, TexturePixelData &out) = 0;
    virtual bool LoadTextureMeta(const std::string &fpath, TexturePixelData &out) = 0;

    virtual TexturePixelData AllocateImage(Device::PixelFormat pf, const emath::usvec2 &Size) = 0;
    virtual void StoreScreenShot(TexturePixelData out) = 0;
};

} //namespace MoonGlare::Renderer::Resources 
