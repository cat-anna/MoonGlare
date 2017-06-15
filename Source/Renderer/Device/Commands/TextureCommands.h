#pragma once

#include "../../Commands/CommandQueueBase.h"
#include "../../Commands/OpenGL/TextureCommands.h"

namespace MoonGlare::Renderer::Device::Commands {

using namespace MoonGlare::Renderer::Commands;

namespace detail {
using namespace MoonGlare::Renderer::Commands::detail;

struct TextureLoadCommandArgument {
    using Common = TextureSettingsCommon<GL_TEXTURE_2D>;

    TextureHandle *m_TextureHandle;
    
    void *m_PixelData;
    Device::ValueFormat m_ValueType;
    Device::PixelFormat m_PixelFormat;
    uint16_t m_Size[2];


    Configuration::TextureLoad m_Configuration;

    void Run() {
        RendererAssert(m_TextureHandle);
        RendererAssert(*m_TextureHandle != 0);

        glBindTexture(GL_TEXTURE_2D, *m_TextureHandle);
        auto bpp = static_cast<unsigned>(m_PixelFormat);
        auto type = static_cast<unsigned>(m_ValueType);
        glTexImage2D(GL_TEXTURE_2D, /*MipmapLevel*/0, bpp, m_Size[0], m_Size[1], /*border*/0, bpp, type, m_PixelData);
        Common::Setup(m_Configuration);
    }
};
}
using TextureLoadCommand = RunnableCommandTemplate<detail::TextureLoadCommandArgument>;


} //namespace MoonGlare::Renderer::Device::Commands
