#pragma once

#include "math/vector.hpp"
#include "renderer/configuration/texture.hpp"
#include "renderer/device_types.hpp"
#include "renderer/renderer_configuration.hpp"
#include "renderer/resources/texture_resource_interface.hpp"
#include "resource_id.hpp"
#include <gsl/gsl>
#include <string>

namespace MoonGlare::Renderer::Resources::Texture {

class iTextureResourceLoader {
public:
    virtual ~iTextureResourceLoader() = default;

    virtual void ScheduleLoad(FileResourceId resource_id, Device::TextureHandle device_handle,
                              math::ivec2 *loaded_image_size,
                              Configuration::TextureLoadConfig load_config = {}) = 0;
};

} // namespace MoonGlare::Renderer::Resources::Texture
