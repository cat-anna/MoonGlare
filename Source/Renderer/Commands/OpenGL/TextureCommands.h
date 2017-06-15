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

struct Texture2DResourceBindArgument : public TextureCommandBase {
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
        case Configuration::Texture::Edges::Default:
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
        case Configuration::Texture::Filtering::Default:
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

} //namespace MoonGlare::Renderer::Commands
