#pragma once

#include "command_common.hpp"
#include "renderer/configuration/texture.hpp"
#include "renderer/device_types.hpp"
#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

//---------------------------------------------------------------------------------------

namespace detail {
struct TextureAllocation {
    using HandleType = Device::TextureHandle;
    static void Allocate(GLsizei count, HandleType *out) { glGenTextures(count, out); }
    static void Release(GLsizei count, HandleType *out) { glDeleteTextures(count, out); }
};
} // namespace detail

using TextureSingleAllocate = detail::SingleAllocate<detail::TextureAllocation>;
using TextureSingleRelease = detail::SingleRelease<detail::TextureAllocation>;
using TextureBulkAllocate = detail::BulkAllocate<detail::TextureAllocation>;
using TextureBulkRelease = detail::BulkRelease<detail::TextureAllocation>;

//---------------------------------------------------------------------------------------

struct Texture2DBindCommand {
    Device::TextureHandle handle;
    void Execute() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, handle);
    }
};

struct Texture2DBindUnitCommand {
    Device::TextureHandle handle;
    GLuint unit_index;
    void Execute() const {
        glActiveTexture(GL_TEXTURE0 + unit_index);
        glBindTexture(GL_TEXTURE_2D, handle);
    }
};

struct TextureCubeBindUnitCommand {
    Device::TextureHandle handle;
    GLuint unit_index;
    void Execute() const {
        glActiveTexture(GL_TEXTURE0 + unit_index);
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
    }
};

//---------------------------------------------------------------------------------------

struct TextureSettingsCommon {
    static void SetupEdges(GLenum texure_mode, Configuration::Texture::Edges Edges);
    static void SetupFiltering(GLenum texure_mode, Configuration::Texture::Filtering Filtering);

    // static void SetupSwizzle(Configuration::Texture::ColorSwizzle swizzle)

    static void Setup(GLenum texure_mode, Configuration::TextureLoadConfig cfg) {
        SetupFiltering(texure_mode, cfg.filtering);
        SetupEdges(texure_mode, cfg.edges);
        // SetupSwizzle(texure_mode, cfg.m_Swizzle);
    }
};

struct Texture2DSetFilteringCommand : public TextureSettingsCommon {
    Configuration::Texture::Filtering filtering;
    void Execute() const { SetupFiltering(GL_TEXTURE_2D, filtering); }
};

struct Texture2DSetEdgesCommand : public TextureSettingsCommon {
    Configuration::Texture::Edges edges;
    void Execute() const { SetupEdges(GL_TEXTURE_2D, edges); }
};

struct Texture2DSetupCommand : public TextureSettingsCommon {
    Configuration::TextureLoadConfig config;
    void Execute() const { Setup(GL_TEXTURE_2D, config); }
};

//---------------------------------------------------------------------------------------

} // namespace MoonGlare::Renderer::Commands

#if 0

namespace MoonGlare::Renderer::Commands{

//---------------------------------------------------------------------------------------

struct Texture2DResourceBindArgument {
    Device::TextureHandle *m_HandlePtr;
    static void Execute(const Texture2DResourceBindArgument *arg) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *m_HandlePtr);
    }
};
using Texture2DResourceBind = CommandTemplate<Texture2DResourceBindArgument>;

struct Texture2DResourceBindUnitArgument : public TextureCommandBase {
    uint16_t unit_index;
    static void Execute(const Texture2DResourceBindUnitArgument *arg) {
        glActiveTexture(GL_TEXTURE0 + unit_index);
        glBindTexture(GL_TEXTURE_2D, *m_HandlePtr);
    }
};
using Texture2DResourceBindUnit = CommandTemplate<Texture2DResourceBindUnitArgument>;

//---------------------------------------------------------------------------------------

struct Texture2DImageArgument {
    GLint m_InternalFormat;
    GLsizei m_Size[2];
    GLenum m_Format;
    GLenum m_Type;
    void *m_Pixels;
    static void Execute(const Texture2DImageArgument *arg) {
        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Size[0], m_Size[1], 0, m_Format, m_Type, m_Pixels);
    }
};
using Texture2DImage = CommandTemplate<Texture2DImageArgument>;

//---------------------------------------------------------------------------------------

template<typename ArgType>
struct Texture2DParameterArgument {
    GLenum m_Parameter;
    ArgType m_Value;
    static void Execute(const Texture2DParameterArgument *arg) {
        Set(m_Parameter, m_Value);
    }

    static void Set(GLenum Parameter, GLint Value) {
        glTexParameteri(GL_TEXTURE_2D, Parameter, Value);
    }
    static void Set(GLenum Parameter, GLfloat Value) {
        glTexParameterf(GL_TEXTURE_2D, Parameter, Value);
    }

    static void Set(GLenum Parameter, const GLint *Value) {
        glTexParameteriv(GL_TEXTURE_2D, Parameter, Value);
    }
    static void Set(GLenum Parameter, const GLfloat *Value) {
        glTexParameterfv(GL_TEXTURE_2D, Parameter, Value);
    }
};

template<typename ArgType>
using Texture2DParameter = CommandTemplate<Texture2DParameterArgument<ArgType>>;

using Texture2DParameterInt = Texture2DParameter<GLint>;
using Texture2DParameterFloat  = Texture2DParameter<GLfloat>;

//---------------------------------------------------------------------------------------


namespace detail {
    struct InitPlaneShadowMapArgument;
}

struct detail::InitPlaneShadowMapArgument {
    using Conf = Configuration::Shadow;

    GLsizei size;
    Device::TextureHandle *textureHandle;
    Device::FramebufferHandle *bufferHandle;

    void Run() {
        if(*bufferHandle == Device::InvalidFramebufferHandle) {
            glGenFramebuffers(1, bufferHandle);
        }
        if (*textureHandle == Device::InvalidTextureHandle) {
            glGenTextures(1, textureHandle);
        }

        glBindTexture(GL_TEXTURE_2D, *textureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
//            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
//            //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
//            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)m_Size[0], (GLsizei)m_Size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
//
//            //	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ShadowTexture, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *bufferHandle);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *textureHandle, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *textureHandle, 0);

        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (Status != GL_FRAMEBUFFER_COMPLETE) {
            AddLogf(Error, "FB error, status: 0x%x\n", Status);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Device::InvalidFramebufferHandle);
    }
};
using InitPlaneShadowMap = RunnableCommandTemplate<detail::InitPlaneShadowMapArgument>;

    //---------------------------------------------------------------------------------------

namespace detail {
struct InitCubeShadowMapArgument;
}

struct detail::InitCubeShadowMapArgument {
    using Conf = Configuration::Shadow;

    GLsizei size;
    Device::TextureHandle *textureHandle;
    Device::FramebufferHandle *bufferHandle;

    void Run() {
        if (*bufferHandle == Device::InvalidFramebufferHandle) {
            glGenFramebuffers(1, bufferHandle);
        }
        if (*textureHandle == Device::InvalidTextureHandle) {
            glGenTextures(1, textureHandle);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, *textureHandle);

        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        //            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        //            //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        //            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)m_Size[0], (GLsizei)m_Size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        //
        //            //	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ShadowTexture, 0);



        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *bufferHandle);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *textureHandle, 0);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *textureHandle, 0);

        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (Status != GL_FRAMEBUFFER_COMPLETE) {
            AddLogf(Error, "FB error, status: 0x%x\n", Status);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Device::InvalidFramebufferHandle);
    }
};
using InitCubeShadowMap = RunnableCommandTemplate<detail::InitCubeShadowMapArgument>;

//---------------------------------------------------------------------------------------

namespace detail {
struct Texture2DSetPixelsArrayArgument;
struct Texture2DGenerateMipMapsArgument;
}

struct detail::Texture2DSetPixelsArrayArgument {
    unsigned short size[2];
    GLenum internalformat;
    GLenum format;
    GLenum type;
    const void *pixels;

    void Run() {
        glTexImage2D(GL_TEXTURE_2D, /*MipmapLevel*/0, internalformat, size[0], size[1], /*border*/0, format, type, pixels);
    }
};
using Texture2DSetPixelsArray = Commands::RunnableCommandTemplate<detail::Texture2DSetPixelsArrayArgument>;


struct detail::Texture2DGenerateMipMapsArgument {
    void Run() {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
};
using Texture2DGenerateMipMapsArgument = Commands::RunnableCommandTemplate<detail::Texture2DGenerateMipMapsArgument>;

//---------------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer::Commands

#endif
