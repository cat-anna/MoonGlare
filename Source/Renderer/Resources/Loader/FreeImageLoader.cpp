#include "pch.h"

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "../TextureResource.h"
#include "FreeImageLoader.h"

#include "../../../Assets/Texture/FreeImageUtils.h"

#include <Renderer/Commands/OpenGL/TextureCommands.h>

namespace MoonGlare::Renderer::Resources::Loader {

void FreeImageLoader::OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) {

    LoadTexture(storage, handle, filedata.get(), filedata.byte_size(), config);
}

void FreeImageLoader::LoadTexture(ResourceLoadStorage &storage, TextureResourceHandle handle, void *image, size_t datasize, Configuration::TextureLoad config) {

    FIMEMORY *fim = FreeImage_OpenMemory((BYTE*)image, datasize);
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);

    int flags = 0;
    switch (fif) {
    case FIF_PNG:
        //flags |= PNG_IGNOREGAMMA;
        break;
    default:
        break;
    }

    FIBITMAP *dib = FreeImage_LoadFromMemory(fif, fim, flags);
    FreeImage_CloseMemory(fim);

    auto dibrelease = ImageUniquePtr((void*)dib, &DibDeallocator);

    LoadImage(storage, dib, fif, handle, config);
}

void FreeImageLoader::LoadImage(ResourceLoadStorage &storage, FIBITMAP *bitmap, FREE_IMAGE_FORMAT fif, TextureResourceHandle handle, Configuration::TextureLoad config) {
    RendererAssert(bitmap);

    switch (fif) {
    //case FIF_PNG: //png images need to be flipped
 //          FreeImage_FlipVertical(bitmap);
        break;
    default:
        //nothing todo
        break;
    }

    //FreeImage_FlipVertical(dib);
    //FreeImage_FlipHorizontal(dib);

    PixelFormat pixelFormat = PixelFormat::RGB8;
    ValueFormat valueFormat = ValueFormat::UnsignedByte;

    auto mask = FreeImage_GetBlueMask(bitmap);
    
    switch (FreeImage_GetBPP(bitmap)) {
    case 32:
        if (mask != FI_RGBA_RED_MASK)
            SwapRedAndBlue(bitmap);
        pixelFormat = PixelFormat::RGBA8;
        break;
    case 24:
        if (mask != FI_RGBA_RED_MASK)
            SwapRedAndBlue(bitmap);
        pixelFormat = PixelFormat::RGB8;
        break;
    default:
    {
        __debugbreak();
        RendererAssert(false);
        //FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib);
        //FreeImage_Unload(dib);
        //dib = dib24;
        //SwapRedAndBlue(dib);
        //out.m_PixelFormat = PixelFormat::RGB8;
    }
    }

    auto size = emath::usvec2(FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap));
    size_t bytesize = (size_t)size[0] * (size_t)size[1] * ((size_t)FreeImage_GetBPP(bitmap) / 8);

    void* pixels = FreeImage_GetBits(bitmap);

    SubmitPixels(storage, pixels, bytesize, size, pixelFormat, valueFormat, handle, config);
}

void FreeImageLoader::SubmitPixels(ResourceLoadStorage &storage, void *pixels, size_t bytesize, const emath::usvec2 &size, PixelFormat pixelFormat, ValueFormat valueFormat, TextureResourceHandle handle, Configuration::TextureLoad config) {
    auto &m = storage.m_Memory.m_Allocator;
    auto &q = storage.m_Queue;

    using namespace Commands;

    //NotEnoughStorage

    void *storagepixels = m.Clone((uint8_t*)pixels, bytesize);
    if (!storagepixels) {
        throw NotEnoughStorage{ bytesize };
    }

    if (*handle.deviceHandle == Device::InvalidTextureHandle)
        q.MakeCommand<TextureSingleAllocate>(handle.deviceHandle);

   q.MakeCommand<Texture2DResourceBind>(handle.deviceHandle);

   auto texdata = q.PushCommand<Texture2DSetPixelsArray>();
   texdata->BPP = static_cast<GLenum>(pixelFormat);
   texdata->pixels = storagepixels;
   texdata->size[0] = size[0];
   texdata->size[1] = size[1];
   texdata->type = static_cast<GLenum>(valueFormat);

   q.MakeCommand<Commands::Texture2DSetup>(config);
   q.MakeCommand<Texture2DBind>(Device::InvalidTextureHandle);
}

} //namespace MoonGlare::Renderer::Resources::Loader 
