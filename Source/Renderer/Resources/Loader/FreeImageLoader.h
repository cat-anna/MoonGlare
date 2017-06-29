#pragma once

struct FIBITMAP;
enum FREE_IMAGE_FORMAT;

namespace MoonGlare::Renderer::Resources::Loader {

class FreeImageLoader : public iAsyncFileSystemRequest {
    using MeshConf = Configuration::Texture;
    using PixelFormat = Asset::TextureLoader::PixelFormat;
    using ValueFormat = Asset::TextureLoader::ValueFormat;
public:
    FreeImageLoader(TextureResourceHandle handle, TextureResource *Owner, Configuration::TextureLoad config) :
        owner(Owner), handle(handle), config(config) {}

    void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage, iAsyncLoader *loader) override;
private:
    TextureResource *owner;
    TextureResourceHandle handle;
    Configuration::TextureLoad config;

    using ImageUniquePtr = std::unique_ptr<void, void(*)(void*)>;

    void LoadFreeImage(ResourceLoadStorage &storage, FIBITMAP *dib, FREE_IMAGE_FORMAT fif);
    void SubmitPixels(ResourceLoadStorage &storage, void *pixels, size_t bytesize, const emath::usvec2 &size, PixelFormat pixelFormat, ValueFormat valueFormat);
};

} //namespace MoonGlare::Renderer::Resources::Loader 
