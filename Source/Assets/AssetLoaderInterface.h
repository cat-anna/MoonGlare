#pragma once

namespace MoonGlare::Asset {

class ShaderCodeLoader {
public:
    virtual ~ShaderCodeLoader() {}

    enum class ShaderType : uint8_t {
        Geometry,
        Vertex,
        Fragment,

        MaxValue,
    };

    struct ShaderCode {
        std::array<std::string, static_cast<size_t>(ShaderType::MaxValue)> m_Code;
    };

    virtual bool LoadCode(const std::string &Name, ShaderCode &Output) { return false; };
};

class TextureLoader {
public:
    virtual ~TextureLoader() {}

    enum class PixelFormat : uint16_t {
        RGB8 = GL_RGB,
        RGBA8 = GL_RGBA,
        GrayScale8 = GL_LUMINANCE, //?
        LuminanceAlpha = GL_LUMINANCE_ALPHA,
    };

    enum class ValueFormat : uint16_t {
        UnsignedByte = GL_UNSIGNED_BYTE,//this constants should not be used here!!!
        Float = GL_FLOAT,
    };

    using ImageUniquePtr = std::unique_ptr<void, void(*)(void*)>;

    struct TexturePixelData {
        void* m_Pixels;
        unsigned m_PixelsByteSize;
        ValueFormat m_PixelType;
        PixelFormat m_PixelFormat;
        emath::usvec2 m_PixelSize;
        ImageUniquePtr m_ImageMemory;
    };

    virtual bool LoadTexture(const std::string &fpath, TexturePixelData &out) = 0;
};

class AssetLoader {
public:
    virtual ~AssetLoader() {}

    virtual ShaderCodeLoader* GetShaderCodeLoader() const { return nullptr; }
    virtual TextureLoader* GetTextureLoader() const { return nullptr; }
};

} //namespace MoonGlare::Renderer::Resources 
