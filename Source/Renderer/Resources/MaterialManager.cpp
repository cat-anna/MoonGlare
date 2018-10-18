#include "ResourceManager.h"
#include "MaterialManager.h"

#include <Renderer/Resources/Texture/TextureResource.h>

#include <Common.x2c.h>
#include <Math.x2c.h>
#include <Material.x2c.h>

namespace MoonGlare::Renderer::Resources {

MaterialManager::MaterialManager(ResourceManager * Owner): m_ResourceManager(Owner) {
    assert(Owner);

    m_AllocationBitmap.ClearAllocation();
    generations.fill(1);
    materials.fill({});
    asyncLoader = Owner->GetLoader();

    assert(asyncLoader);
}

MaterialManager::~MaterialManager() {
}

//---------------------------------------------------------------------------------------

Material *MaterialManager::GetMaterial(MaterialResourceHandle h) {
    if (!IsHandleValid(h))
        return nullptr;
    return &materials[h.index];
}

bool MaterialManager::IsHandleValid(MaterialResourceHandle &h) const {
    if (h.index >= Conf::Limit)
        return false;
    if (generations[h.index] != h.generation) {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------

MaterialResourceHandle MaterialManager::LoadMaterial(const std::string &uri) {
    MaterialResourceHandle h;    
    switch (Allocate(h, uri)) {
    case AllocateResult::Success:
        ReloadMaterial(uri, h);
        [[fallthrough]];
    case AllocateResult::CacheHit:
        return h;
    case AllocateResult::Failure:
    default:
        return { };
    }
}

void MaterialManager::ReloadMaterial(const std::string &uri, MoonGlare::Renderer::MaterialResourceHandle h) {
    asyncLoader->QueueRequest(uri, [this, h](const std::string &uri, StarVFS::ByteTable &data, ResourceLoadStorage &storage) {
        MaterialTemplate matData;
        pugi::xml_document xdoc;
        xdoc.load(data.c_str());
        MoonGlare::x2c::Renderer::MaterialTemplate_t_Read(xdoc.document_element(), matData, nullptr);

        ApplyTemplate(h, matData);
        AddLogf(Debug, "Loaded material %s", uri.c_str());
    });
}

MaterialResourceHandle MaterialManager::CreateMaterial(const std::string &uri, const MaterialTemplate &matTemplate) {
    MaterialResourceHandle h;
    if (Allocate(h, uri) == AllocateResult::Failure)
        return {};
    ApplyTemplate(h, matTemplate);
    return h;
}

void MaterialManager::ApplyTemplate(MaterialResourceHandle handle, const MaterialTemplate &matTemplate) {
    if (!IsHandleValid(handle))
        return;

    auto &mat = materials[handle.index];

    mat.diffuseColor = emath::MathCast<emath::fvec3>(matTemplate.diffuseColor);
    mat.specularColor = emath::MathCast<emath::fvec3>(matTemplate.specularColor);
    mat.emissiveColor = emath::MathCast<emath::fvec3>(matTemplate.emissiveColor);
    mat.shiness = matTemplate.shiness;

    auto loadMap = [this, &mat](Material::MapType type, const MaterialTemplate::Map &map) {
        if (!map.enabled)
            return;

        auto &texR = m_ResourceManager->GetTextureResource();
        bool handleValid = texR.IsHandleValid(map.textureHandle);
        if (!map.texture.empty() && handleValid) {
            AddLogf(Warning, "Texture uri and texture handle specified at the same time, handle will be used");
            return;
        }

        size_t index = (size_t)type;
        mat.mapEnabled[index] = false;
        mat.mapTexture[index] = { };

        if (handleValid) {
            mat.mapEnabled[index] = true;
            mat.mapTexture[index] = map.textureHandle;
            return;
        }

        if (!map.texture.empty()) {
            mat.mapEnabled[index] = true;
            auto cfg = map.cfg;
            cfg.m_Flags.useSRGBColorSpace = type == Material::MapType::Diffuse || type == Material::MapType::Specular;
            mat.mapTexture[index] = texR.LoadTexture(map.texture, cfg);
            return;
        }
    };

    loadMap(Material::MapType::Diffuse, matTemplate.diffuseMap);
    loadMap(Material::MapType::Normal, matTemplate.normalMap);
    loadMap(Material::MapType::Specular, matTemplate.specularMap);
    loadMap(Material::MapType::Shiness, matTemplate.shinessMap);
}
 
//---------------------------------------------------------------------------------------

MaterialManager::AllocateResult MaterialManager::Allocate(MaterialResourceHandle &hout, const std::string &uri) {
    if (!uri.empty()) {
        auto cache = loadedMaterials.find(uri);
        if (cache != loadedMaterials.end() && IsHandleValid(cache->second)) {
            AddLogf(Performance, "material cache hit [%s]", uri.c_str());
            hout = cache->second;
            return AllocateResult::CacheHit;
        }
    }

    if (Allocate(hout) == AllocateResult::Success) {
        if (!uri.empty())
            loadedMaterials[uri] = hout;
        return AllocateResult::Success;
    }

    return AllocateResult::Failure;
}

MaterialManager::AllocateResult MaterialManager::Allocate(MaterialResourceHandle &hout) {
    Bitmap::Index_t index;
    if (m_AllocationBitmap.Allocate(index)) {
        hout.index = static_cast<MaterialResourceHandle::Index_t>(index);
        hout.generation = generations[hout.index];
        hout.deviceHandle = &materials[hout.index];
        return AllocateResult::Success;
    }
    else {
        AddLogf(Debug, "material allocation failed");
        return AllocateResult::Failure;
    }
}

//---------------------------------------------------------------------------------------

void MaterialManager::ReloadMaterials() {
    for (auto&[uri,h] : loadedMaterials) {
        ReloadMaterial(uri, h);
    }
}

} //namespace MoonGlare::Renderer::Resources 
