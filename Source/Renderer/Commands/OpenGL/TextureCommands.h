#pragma once

#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands{

struct Texture2DBindArgument {
    Device::TextureHandle m_Texture;
    static void Execute(const Texture2DBindArgument *arg) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, arg->m_Texture);
    }
};
using Texture2DBind = CommandTemplate<Texture2DBindArgument>;

struct Texture2DBindUnitArgument {
    Device::TextureHandle m_Texture;
    unsigned m_UnitIndex;
    static void Execute(const Texture2DBindUnitArgument *arg) {
        glActiveTexture(GL_TEXTURE0 + arg->m_UnitIndex);
        glBindTexture(GL_TEXTURE_2D, arg->m_Texture);
    }
};
using Texture2DBindUnit = CommandTemplate<Texture2DBindUnitArgument>;

//---------------------------------------------------------------------------------------

struct Texture2DResourceBindArgument {
    Device::TextureHandle *m_HandlePtr;
    static void Execute(const Texture2DResourceBindArgument *arg) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *arg->m_HandlePtr);
    }
};
using Texture2DResourceBind = CommandTemplate<Texture2DResourceBindArgument>;

struct Texture2DResourceBindUnitArgument : public TextureCommandBase {
    uint16_t m_UnitIndex;
    static void Execute(const Texture2DResourceBindUnitArgument *arg) {
        glActiveTexture(GL_TEXTURE0 + arg->m_UnitIndex);
        glBindTexture(GL_TEXTURE_2D, *arg->m_HandlePtr);
    }
};
using Texture2DResourceBindUnit = CommandTemplate<Texture2DResourceBindUnitArgument>;

//---------------------------------------------------------------------------------------

namespace detail {
struct TextureAllocation {
    using Handle_t = Device::TextureHandle;
    static void Allocate(GLsizei count, Handle_t *out) {
        glGenTextures(count, out);
    }
    static void Release(GLsizei count, Handle_t *out) {
        glDeleteTextures(count, out);
    }
};
}

using TextureSingleAllocate = CommandTemplate<detail::SingleAllocate <detail::TextureAllocation> >;
using TextureSingleRelease = CommandTemplate<detail::SingleRelease <detail::TextureAllocation> >;
using TextureBulkAllocate = CommandTemplate<detail::BulkAllocate <detail::TextureAllocation> >;
using TextureBulkRelease = CommandTemplate<detail::BulkRelease <detail::TextureAllocation> >;

//---------------------------------------------------------------------------------------

struct Texture2DImageArgument {
    GLint m_InternalFormat;
    GLsizei m_Size[2];
    GLenum m_Format;
    GLenum m_Type;
    void *m_Pixels;
    static void Execute(const Texture2DImageArgument *arg) {
        glTexImage2D(GL_TEXTURE_2D, 0, arg->m_InternalFormat, arg->m_Size[0], arg->m_Size[1], 0, arg->m_Format, arg->m_Type, arg->m_Pixels);
    }
};
using Texture2DImage = CommandTemplate<Texture2DImageArgument>;

//---------------------------------------------------------------------------------------

template<typename ArgType>
struct Texture2DParameterArgument {
    GLenum m_Parameter;
    ArgType m_Value;
    static void Execute(const Texture2DParameterArgument *arg) {
        Set(arg->m_Parameter, arg->m_Value);
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

template <GLenum TEXTUREMODE>
struct TextureSettingsCommon {
    static void GenerateMipmaps() {
        glGenerateMipmap(TEXTUREMODE); 
    }

    static void SetNearestFiltering() {
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    static void SetLinearFiltering() {
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    static void SetBilinearFiltering() {
        GenerateMipmaps();
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    }

    static void SetTrilinearFiltering() {
        GenerateMipmaps();
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    static void SetClampToEdges() {
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    static void SetRepeatEdges() {
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(TEXTUREMODE, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_REPEAT);
    }

    static void SetupEdges(Configuration::Texture::Edges Edges) {
        switch (Edges) {
        case Configuration::Texture::Edges::Repeat:
            SetRepeatEdges();
            break;
        default:
            AddLogf(Error, "Unknown edges mode!");
        case Configuration::Texture::Edges::Clamp:
            SetClampToEdges();
        }
    }

    static void SetupFiltering(Configuration::Texture::Filtering Filtering) {
        switch (Filtering) {
        case Configuration::Texture::Filtering::Bilinear:
            SetBilinearFiltering();
            break;
        case Configuration::Texture::Filtering::Trilinear:
            SetTrilinearFiltering();
            break;
        case Configuration::Texture::Filtering::Nearest:
            SetNearestFiltering();
            break;
        default:
            AddLogf(Error, "Unknown filtering mode!");
        case Configuration::Texture::Filtering::Linear:
            SetLinearFiltering();
        }
    }

    static void Setup(Configuration::TextureLoad cfg) {
        SetupFiltering(cfg.m_Filtering);
        SetupEdges(cfg.m_Edges);
    }
};

struct Texture2DSetFilteringArgument;
struct Texture2DSetEdgesArgument;
struct Texture2DSetupArgument;
}

struct detail::Texture2DSetFilteringArgument {
    using Common = TextureSettingsCommon<GL_TEXTURE_2D>;

    Configuration::Texture::Filtering m_Filtering;

    void Run() {
        Common::SetupFiltering(m_Filtering);
    }
};
using Texture2DSetFiltering = RunnableCommandTemplate<detail::Texture2DSetFilteringArgument>;

struct detail::Texture2DSetEdgesArgument {
    using Common = TextureSettingsCommon<GL_TEXTURE_2D>;

    Configuration::Texture::Edges m_Edges;

    void Run() {
        Common::SetupEdges(m_Edges);
    }
};
using Texture2DSetEdges = RunnableCommandTemplate<detail::Texture2DSetEdgesArgument>;

struct detail::Texture2DSetupArgument {
    using Common = TextureSettingsCommon<GL_TEXTURE_2D>;

    Configuration::TextureLoad m_Config;

    void Run() {
        Common::Setup(m_Config);
    }
};
using Texture2DSetup = RunnableCommandTemplate<detail::Texture2DSetupArgument>;

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *textureHandle, 0);

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
