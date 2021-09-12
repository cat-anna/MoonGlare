#pragma once

#include "renderer/types.hpp"
#include "resource_id.hpp"

namespace MoonGlare::Renderer::Resources {

class iTextureResource {
public:
    virtual ~iTextureResource() = default;

    // virtual void ReloadAllTextures() = 0;

    // must match with iRuntimeResourceLoader (double override by real implementation)
    virtual ResourceHandle LoadTextureResource(FileResourceId file_id) = 0;

    virtual TextureHandle LoadTexture(const std::string &name) = 0;
};

} // namespace MoonGlare::Renderer::Resources
