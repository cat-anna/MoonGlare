#pragma once

struct FIBITMAP;
enum FREE_IMAGE_FORMAT;

namespace MoonGlare::Renderer::Resources::Texture {

class FreeImageLoader : public iAsyncFileSystemRequest {
    using MeshConf = Configuration::Texture;
    using PixelFormat = Device::PixelFormat;
    using ValueFormat = Device::ValueFormat;
public:
    FreeImageLoader(TextureResourceHandle handle, TextureResource *Owner, Configuration::TextureLoad config) :
        owner(Owner), handle(handle), config(config) {}

    void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override;

    void LoadTexture(ResourceLoadStorage &storage, void *image, size_t datasize);
private:
    TextureResource *owner;
    TextureResourceHandle handle;
    Configuration::TextureLoad config;

    using ImageUniquePtr = std::unique_ptr<void, void(*)(void*)>;

    void LoadImage(ResourceLoadStorage &storage, FIBITMAP *dib, FREE_IMAGE_FORMAT fif);
};

} 
