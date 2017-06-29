/*
  * Generated by cppsrc.sh
  * On 2017-02-13 22:45:35,93
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "../nfRenderer.h"
#include "../Frame.h"
#include "../Renderer.h"
#include "../RenderDevice.h"

#include "../Commands/CommandQueue.h"
#include "../Commands/OpenGL/TextureCommands.h"
#include "../Commands/OpenGL/TextureInitCommands.h"

#include "ResourceManager.h"
#include "TextureResource.h"
#include "AsyncLoader.h"

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Resources/ResourceManager.h>

namespace MoonGlare::Renderer::Resources {

void TextureResource::Initialize(ResourceManager *Owner, Asset::TextureLoader *TexLoader) {
    RendererAssert(Owner);
    RendererAssert(TexLoader);
    m_ResourceManager = Owner;
    m_Settings = &Owner->GetConfiguration()->m_Texture;

    m_GLHandle.fill(Device::InvalidTextureHandle);
    m_TextureSize.fill(emath::usvec2(0));
    m_AllocationBitmap.ClearAllocation();

    if (Conf::Initial > 0) {
        //TBD!!!
    }
}

void TextureResource::Finalize() {
}

//---------------------------------------------------------------------------------------

bool TextureResource::Allocate(Frame *frame, TextureResourceHandle &out) {
    RendererAssert(frame);
    return Allocate(&frame->GetControllCommandQueue(), out);
}

bool TextureResource::Allocate(Commands::CommandQueue *queue, TextureResourceHandle &out) {
    Bitmap::Index_t index;
    if (m_AllocationBitmap.Allocate(index)) {
        if (queue && m_GLHandle[index] == Device::InvalidTextureHandle) {
            IncrementPerformanceCounter(OpenGLAllocations);
            auto arg = queue->PushCommand<Commands::TextureSingleAllocate>();
            arg->m_Out = &m_GLHandle[index];
        }
        out.m_Index = static_cast<TextureResourceHandle::Index_t>(index);
        out.m_TmpGuard = out.GuardValue;
        IncrementPerformanceCounter(SuccessfulAllocations);
        return true;
    }
    else {
        AddLogf(Debug, "Texture allocation failed");
        IncrementPerformanceCounter(FailedAllocations);
        return false;
    }
}

void TextureResource::Release(Frame *frame, TextureResourceHandle h) {
    RendererAssert(h.m_TmpGuard == h.GuardValue);
    RendererAssert(h.m_Index < Conf::Limit);

    if (m_AllocationBitmap.Release(h.m_Index)) {
        IncrementPerformanceCounter(SuccessfulDellocations);
    }
    else {
        AddLogf(Debug, "Texture deallocation failed");
        IncrementPerformanceCounter(FailedDellocations);
    }
}

//---------------------------------------------------------------------------------------

bool TextureResource::LoadTexture(TextureResourceHandle &out, const std::string &fPath, Configuration::TextureLoad config, bool CanAllocate) {
    if (!out && !CanAllocate) {
        return false;
    }

    if (!out && CanAllocate) {
        if (!Allocate((Commands::CommandQueue*)nullptr, out)) {
            DebugLogf(Error, "texture allocation - allocate failed");
            return false;
        }
    } 

    auto *rawptr = &m_GLHandle[out.m_Index];
    auto loader = m_ResourceManager->GetLoader();
    auto *size = &m_TextureSize[out.m_Index];

    loader->SubmitTextureLoad(
        fPath,//?
        out,
        rawptr,
        size,
        config
    );

    return true;
}

emath::usvec2 TextureResource::GetSize(TextureResourceHandle h) const {
    if (!h)
        return {};

    return m_TextureSize[h.m_Index];
}

//---------------------------------------------------------------------------------------

bool TextureResource::SetTexturePixels(TextureResourceHandle & out, Commands::CommandQueue & q, const void * Pixels, const emath::usvec2 & size, Configuration::TextureLoad config, Device::PixelFormat pxtype, bool AllowAllocate, Commands::CommandKey key, uint16_t TypeValue, uint16_t ElementSize) {
    if (!out && AllowAllocate) {
        if (!Allocate(&q, out)) {
            DebugLogf(Error, "texture allocation - allocate failed");
            return false;
        }
    }

    auto texres = q.PushCommand<Commands::Texture2DResourceBind>(key);
    texres->m_HandlePtr = GetHandleArrayBase() + out.m_Index;

    m_TextureSize[out.m_Index] = size;
    auto pixels = q.PushCommand<Commands::Texture2DSetPixelsArray>(key);
    pixels->m_Size[0] = size[0];
    pixels->m_Size[1] = size[1];
    pixels->m_PixelData = Pixels;
    pixels->m_BPP = static_cast<GLenum>(pxtype);
    pixels->m_Type = static_cast<GLenum>(TypeValue);

    if (config.m_Filtering == Conf::Filtering::Default) {
        config.m_Filtering = m_Settings->m_Filtering;
    }
    q.PushCommand<Commands::Texture2DSetup>(key)->m_Config = config;

    return true;
}

} //namespace MoonGlare::Renderer::Resources 
        