#include "texture_commands.hpp"
#include <orbit_logger.h>

namespace MoonGlare::Renderer::Commands {

namespace {

inline void GenerateMipmaps(GLenum texure_mode) {
    glGenerateMipmap(texure_mode);
}

inline void SetNearestFiltering(GLenum texure_mode) {
    glTexParameteri(texure_mode, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(texure_mode, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

inline void SetLinearFiltering(GLenum texure_mode) {
    glTexParameteri(texure_mode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texure_mode, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

inline void SetBilinearFiltering(GLenum texure_mode) {
    GenerateMipmaps(texure_mode);
    glTexParameteri(texure_mode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texure_mode, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

inline void SetTrilinearFiltering(GLenum texure_mode) {
    GenerateMipmaps(texure_mode);
    glTexParameteri(texure_mode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texure_mode, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

inline void SetClampToEdges(GLenum texure_mode) {
    glTexParameteri(texure_mode, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(texure_mode, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

inline void SetRepeatEdges(GLenum texure_mode) {
    glTexParameteri(texure_mode, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texure_mode, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_REPEAT);
}

} // namespace

void TextureSettingsCommon::SetupEdges(GLenum texure_mode, Configuration::Texture::Edges Edges) {
    switch (Edges) {
    case Configuration::Texture::Edges::Repeat:
        SetRepeatEdges(texure_mode);
        break;
    default:
        AddLogf(Error, "Unknown edges mode!");
    case Configuration::Texture::Edges::Clamp:
        SetClampToEdges(texure_mode);
    }
}

void TextureSettingsCommon::SetupFiltering(GLenum texure_mode,
                                           Configuration::Texture::Filtering Filtering) {
    switch (Filtering) {
    case Configuration::Texture::Filtering::Bilinear:
        SetBilinearFiltering(texure_mode);
        break;
    case Configuration::Texture::Filtering::Trilinear:
        SetTrilinearFiltering(texure_mode);
        break;
    case Configuration::Texture::Filtering::Nearest:
        SetNearestFiltering(texure_mode);
        break;
    default:
        AddLogf(Error, "Unknown filtering mode!");
    case Configuration::Texture::Filtering::Linear:
        SetLinearFiltering(texure_mode);
    }
}

// void TextureSettingsCommon::SetupSwizzle(Configuration::Texture::ColorSwizzle swizzle) {
//     if (!swizzle.enable) {
//         return;
//     }
//     glTexParameteri(texure_mode, GL_TEXTURE_SWIZZLE_R,
//                     Configuration::Texture::ChannelSwizzleToEnum(swizzle.R));
//     glTexParameteri(texure_mode, GL_TEXTURE_SWIZZLE_G,
//                     Configuration::Texture::ChannelSwizzleToEnum(swizzle.G));
//     glTexParameteri(texure_mode, GL_TEXTURE_SWIZZLE_B,
//                     Configuration::Texture::ChannelSwizzleToEnum(swizzle.B));
//     glTexParameteri(texure_mode, GL_TEXTURE_SWIZZLE_A,
//                     Configuration::Texture::ChannelSwizzleToEnum(swizzle.A));
// }

} //namespace MoonGlare::Renderer::Commands
