#pragma once

#include "Handles.h"
#include <Foundation/EnumArray.h>

#include "Configuration.Renderer.h"

namespace MoonGlare::Renderer {

struct Material {
    enum class MapType : uint8_t {
        Diffuse, 
        Normal,
        Shiness, //specular exponent
        Specular,     
        MaxValue,
    };

    template<typename T>
    using Array = EnumArray<MapType, T, (size_t)MapType::MaxValue>;

    emath::fvec3 diffuseColor;
    emath::fvec3 specularColor;
    emath::fvec3 emissiveColor;

    Array<bool> mapEnabled;
    Array<TextureResourceHandle> mapTexture;

    float shiness;

	void Reset() {
        diffuseColor = emath::fvec3(1);
        specularColor = emath::fvec3(1);
        emissiveColor = emath::fvec3(0);

        shiness = 16.0f / 128.0f;

        mapEnabled.fill(false);
        mapTexture.fill({});
	}
};

struct MaterialTemplate {
    struct Map {
        bool enabled = false;
        std::string texture;
        TextureResourceHandle textureHandle = { 0 };

        Configuration::TextureLoad cfg = Configuration::TextureLoad::Default();
        Configuration::Texture::Edges &edges = cfg.m_Edges; //TODO: this is ugly workaround

        //TODO: x2c does not support std::variant
        //std::variant<std::nullopt_t , std::string, TextureResourceHandle> texture;
    };

    math::fvec3 diffuseColor = math::fvec3{ 1,1,1, };
    math::fvec3 specularColor = math::fvec3{ 1,1,1, };
    math::fvec3 emissiveColor = math::fvec3{ 0,0,0, };

    float shiness = 16.0f / 128.0f;

    Map diffuseMap;
    Map normalMap;
    Map specularMap;
    Map shinessMap;
};

//static_assert((sizeof(Material) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<Material>::value, "Must be trivial");

} //namespace MoonGlare::Renderer
