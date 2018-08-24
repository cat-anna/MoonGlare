#include "ResourceManager.h"
#include "MaterialManager.h"

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
    if (!Allocate(h, uri))
        return {};

    asyncLoader->QueueRequest(uri, [this, h](const std::string &uri, StarVFS::ByteTable &data, ResourceLoadStorage &storage) {
        MaterialTemplate matData;
        pugi::xml_document xdoc;
        xdoc.load(data.c_str());
        MoonGlare::x2c::Renderer::MaterialTemplate_t_Read(xdoc.document_element(), matData, nullptr);

        ApplyTemplate(h, matData);
    });

    return h;
}

MaterialResourceHandle MaterialManager::CreateMaterial(const std::string &uri, const MaterialTemplate &matTemplate) {
    MaterialResourceHandle h;
    if (!Allocate(h, uri))
        return {};
    ApplyTemplate(h, matTemplate);
    return h;
}

void MaterialManager::ApplyTemplate(MaterialResourceHandle handle, const MaterialTemplate &matTemplate) {
    if (!IsHandleValid(handle))
        return;

    Configuration::TextureLoad config = Configuration::TextureLoad::Default();

    auto &mat = materials[handle.index];

    mat.diffuseColor = emath::MathCast<emath::fvec4>(matTemplate.diffuseColor);

    auto loadMap = [this, &mat, &config](Material::MapType type, const MaterialTemplate::Map &map) {
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
            config.m_Flags.useSRGBColorSpace = type == Material::MapType::Diffuse;
            config.m_Edges = map.edges;
            mat.mapTexture[index] = texR.LoadTexture(map.texture, config);
            return;
        }
    };

    loadMap(Material::MapType::Diffuse, matTemplate.diffuseMap);
}
 
//---------------------------------------------------------------------------------------

bool MaterialManager::Allocate(MaterialResourceHandle &hout, const std::string &uri) {
    if (!uri.empty()) {
        auto cache = loadedMaterials.find(uri);
        if (cache != loadedMaterials.end() && IsHandleValid(cache->second)) {
            AddLogf(Performance, "material load cache hit");
            hout = cache->second;
            return true;
        }
    }

    if (Allocate(hout)) {
        if (!uri.empty())
            loadedMaterials[uri] = hout;
        return true;
    }

    return false;
}

bool MaterialManager::Allocate(MaterialResourceHandle &hout) {
    Bitmap::Index_t index;
    if (m_AllocationBitmap.Allocate(index)) {

        hout.index = static_cast<MaterialResourceHandle::Index_t>(index);
        hout.generation = generations[hout.index];
        hout.deviceHandle = &materials[hout.index];

        return true;
    }
    else {
        AddLogf(Debug, "material allocation failed");
        return false;
    }
}

//---------------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer::Resources 
