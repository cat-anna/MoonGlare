#include "../../nfRenderer.h"
#include "../../Frame.h"
#include "../../Renderer.h"
#include "../../RenderDevice.h"

#include "../../Commands/CommandQueue.h"
#include "../../Commands/OpenGL/TextureCommands.h"

#include "../ResourceManager.h"
#include "TextureResource.h"
#include "../AsyncLoader.h"

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Resources/ResourceManager.h>            

#include "FreeImageLoader.h"

namespace MoonGlare::Renderer::Resources {

void TextureResource::Initialize(ResourceManager *Owner, iFileSystem *fileSystem) {
    assert(Owner);
    assert(fileSystem);
    m_ResourceManager = Owner;
    m_Settings = &Owner->GetConfiguration()->texture;

    m_GLHandle.fill(Device::InvalidTextureHandle);
    m_TextureSize.fill(emath::usvec2(0,0));
    m_AllocationBitmap.ClearAllocation();
    generations.fill(1);


    uint32_t index;
    m_AllocationBitmap.Allocate(index);
    assert(index == 0); //TODO
}

void TextureResource::Finalize() {
}

//---------------------------------------------------------------------------------------

//#ifdef DEBUG_API
TextureResourceHandle TextureResource::AllocExtTexture(const std::string &uri, Device::TextureHandle devTex, const emath::usvec2 &size) {
    Bitmap::Index_t index;
    if (m_AllocationBitmap.Allocate(index)) {
        TextureResourceHandle out;
        out.index = static_cast<TextureResourceHandle::Index_t>(index);
        out.generation = generations[out.index];
        out.deviceHandle = &m_GLHandle[out.index];
        m_TextureSize[out.index] = size;
        *out.deviceHandle = devTex;
        loadedTextures[uri] = out;
        return out;
    } else {
        AddLogf(Debug, "Texture allocation failed");
        return {};
    }
}
//#endif

bool TextureResource::Allocate(TextureResourceHandle &out) {
    Bitmap::Index_t index;
    if (m_AllocationBitmap.Allocate(index)) {
        out.index = static_cast<TextureResourceHandle::Index_t>(index);
        out.generation = generations[out.index];
        out.deviceHandle = &m_GLHandle[out.index];
        return true;
    }
    else {
        AddLogf(Debug, "Texture allocation failed");
        return false;
    }
}

void TextureResource::Release(TextureResourceHandle h) {
    if (!IsHandleValid(h))
        return;

    if (m_AllocationBitmap.Release(h.index)) {
        generations[h.index]++;
    }
    else {
        AddLogf(Debug, "Texture deallocation failed");
    }
}

//---------------------------------------------------------------------------------------

bool TextureResource::LoadTexture(TextureResourceHandle &hout, const std::string &uri, Configuration::TextureLoad config, bool CanAllocate) {
    auto cache = loadedTextures.find(uri);
    if (cache != loadedTextures.end() && IsHandleValid(cache->second)) {
        AddLogf(Debug, "texture load cache hit");
        hout = cache->second;
        return true;
    }

    if (!IsHandleValid(hout)) {
        if (!CanAllocate) {
            return false;
        }
        if (!Allocate(hout)) {
            DebugLogf(Error, "texture allocation - allocate failed");
            return false;
        }
    }

    config.Check(*m_Settings);
    auto loaderif = m_ResourceManager->GetLoader();
    loaderif->QueueRequest(uri, std::make_shared<Texture::FreeImageLoader>(hout, this, config));

    loadedTextures[uri] = hout;

    return true;
}

emath::usvec2 TextureResource::GetSize(TextureResourceHandle h) const {
    if (!IsHandleValid(h))
        return {0, 0};

    return m_TextureSize[h.index];
}

bool TextureResource::IsHandleValid(TextureResourceHandle h) const {
    if (h.index >= Conf::Limit)
        return false;
    if (generations[h.index] != h.generation) {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------

bool TextureResource::SetTexturePixels(TextureResourceHandle & out, Commands::CommandQueue & q, const void * Pixels, const emath::usvec2 & size, 
            Configuration::TextureLoad config, Device::PixelFormat internalformat, Device::PixelFormat format, bool AllowAllocate, Device::ValueFormat TypeValue, Commands::CommandKey key) {
    if (!IsHandleValid(out) && AllowAllocate) {
        if (!Allocate(out)) {
            DebugLogf(Error, "texture allocation - allocate failed");
            return false;
        }
    }

    if (*out.deviceHandle == Device::InvalidTextureHandle)
        q.MakeCommand<Commands::TextureSingleAllocate>(out.deviceHandle);

    auto texres = q.PushCommand<Commands::Texture2DResourceBind>(key);
    texres->m_HandlePtr = GetHandleArrayBase() + out.index;

    m_TextureSize[out.index] = size;
    auto pixels = q.PushCommand<Commands::Texture2DSetPixelsArray>(key);
    pixels->size[0] = size[0];
    pixels->size[1] = size[1];
    pixels->pixels = Pixels;
    pixels->internalformat = static_cast<GLenum>(internalformat);
    pixels->format = static_cast<GLenum>(format);
    pixels->type = static_cast<GLenum>(TypeValue);

    config.Check(*m_Settings);
    q.PushCommand<Commands::Texture2DSetup>(key)->m_Config = config;

    q.MakeCommand<Commands::Texture2DBind>(Device::InvalidTextureHandle);

    return true;
}

} //namespace MoonGlare::Renderer::Resources 
        