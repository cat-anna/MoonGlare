#pragma once

#include "renderer/device_types.hpp"
#include "texture_commands.hpp"
#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

struct TextureInitDefaultTexture {
    void Execute() const {
        static const unsigned char kDefaultTextureBytes[] = {255, 255, 255};
        glBindTexture(GL_TEXTURE_2D, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     kDefaultTextureBytes);
    }
};

struct TextureLoadCommandCommand {
    // using Common = TextureSettingsCommon<GL_TEXTURE_2D>;

    Device::TextureHandle texture_handle;

    void *pixels;
    GLsizei size[2];
    Device::ValueFormat value_format;
    Device::PixelFormat pixel_format;

    Configuration::TextureLoadConfig config;

    void Execute() const {
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        auto source_format = static_cast<GLenum>(pixel_format);
        auto type = static_cast<GLenum>(value_format);

        auto device_format = source_format;

        glTexImage2D(GL_TEXTURE_2D, /*MipmapLevel*/ 0, //
                     device_format,                    //
                     size[0], size[1],                 //
                     /*border*/ 0,                     //
                     source_format, type,              //
                     pixels);

        TextureSettingsCommon::Setup(GL_TEXTURE_2D, config);
    }
};

} // namespace MoonGlare::Renderer::Commands
