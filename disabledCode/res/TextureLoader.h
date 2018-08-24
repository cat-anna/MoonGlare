#pragma once

#include <Foundation/iFileSystem.h>
#include "LoaderInterface.h"

namespace MoonGlare::Renderer::Resources::Texture {

class Loader final : public TextureLoader {
public:
    Loader(iFileSystem *fs);
    ~Loader();

    void Initialize();
    void Finalize();

    bool LoadTextureMemory(const void* ImgData, unsigned ImgLen, TexturePixelData &out, bool LoadPixels);
    bool LoadTextureURI(const std::string &URI, TexturePixelData &out, bool LoadPixels);

    virtual bool LoadTexture(const std::string &fpath, TexturePixelData &out) override;
    virtual bool LoadTextureMeta(const std::string &fpath, TexturePixelData &out) override;

    virtual TexturePixelData AllocateImage(Device::PixelFormat pf, const emath::usvec2 &Size) override;

    virtual void StoreScreenShot(TexturePixelData out) override;
protected:
    iFileSystem* GetFileSystem() { return m_FileSystem; }
private: 
    iFileSystem *m_FileSystem;
};

} 