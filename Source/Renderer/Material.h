#pragma once

#include "Handles.h"

#include "Configuration.Renderer.h"

namespace MoonGlare::Renderer {

struct Material {
    enum class MapType : uint8_t {
        Diffuse, 
        MaxValue,
    };

    template<typename T>
    using Array = std::array<T, (size_t)MapType::MaxValue>;

    emath::fvec4 diffuseColor;
    //emath::fvec4 specularColor;

    Array<bool> mapEnabled;
    Array<TextureResourceHandle> mapTexture;

    //float shines;

	void Reset() {
        diffuseColor = emath::fvec4(1);
        //specularColor = emath::fvec4(1);
        //shines = 32;

        //mapEnabled.fill(false);
        mapTexture.fill({});
	}
};

struct MaterialTemplate {
    struct Map {
        bool enabled = false;
        Configuration::Texture::Edges edges = Configuration::Texture::Edges::Default;
        std::string texture;
        TextureResourceHandle textureHandle = {};
        //TODO: x2c does not support std::variant
        //std::variant<std::nullopt_t , std::string, TextureResourceHandle> texture;
    };

    math::fvec4 diffuseColor = math::fvec4{ 1,1,1,1 };
    math::fvec4 specularColor = math::fvec4{ 1,1,1,1 };

    float shiness;

    Map diffuseMap;
    Map normalMap;
    Map specularMap;
    Map shinessMap;
};

//static_assert((sizeof(Material) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<Material>::value, "Must be trivial");

} //namespace MoonGlare::Renderer
