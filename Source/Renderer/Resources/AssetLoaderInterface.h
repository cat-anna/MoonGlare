#pragma once

#include "../Device/Types.h"

namespace MoonGlare::Renderer::Resources {

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

    using PixelType = Device::ValueFormat;
    using PixelFormat = Device::PixelFormat;

    using ImageUniquePtr = std::unique_ptr<void, void(*)(void*)>;

    struct TexturePixelData {
        void* m_Pixels;
        unsigned m_PixelsByteSize;
        PixelType m_PixelType;
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
