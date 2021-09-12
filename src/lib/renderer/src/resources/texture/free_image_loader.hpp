#pragma once

#include "async_loader.hpp"
#include "command_queue.hpp"
#include "renderer/configuration/texture.hpp"
#include "renderer/renderer_configuration.hpp"
#include "renderer/resources.hpp"
#include "texture_loader_interface.hpp"
#include <gsl/gsl>
#include <memory>

struct FIBITMAP;
enum FREE_IMAGE_FORMAT;

namespace MoonGlare::Renderer::Resources::Texture {

class FreeImageLoader : public iTextureResourceLoader {
public:
    FreeImageLoader(gsl::not_null<iAsyncLoader *> async_loader,
                    gsl::not_null<iContextResourceLoader *> context_loader,
                    gsl::not_null<Configuration::TextureGlobalConfig *> global_config)
        : global_config(global_config), async_loader(async_loader), context_loader(context_loader) {
    }
    ~FreeImageLoader() override = default;

    void ScheduleLoad(FileResourceId resource_id, Device::TextureHandle device_handle,
                      math::ivec2 *loaded_image_size,
                      Configuration::TextureLoadConfig load_config = {}) override;

public:
    Configuration::TextureGlobalConfig *const global_config;
    iAsyncLoader *const async_loader;
    iContextResourceLoader *const context_loader;
};

class FreeImageLoaderRequest : public std::enable_shared_from_this<FreeImageLoaderRequest>,
                               public iContextResourceTask {
public:
    FreeImageLoaderRequest(gsl::not_null<iContextResourceLoader *> context_loader,
                           Device::TextureHandle device_handle, math::ivec2 *loaded_image_size,
                           Configuration::TextureLoadConfig load_config)
        : context_loader(context_loader), loaded_image_size(loaded_image_size),
          device_handle(device_handle), load_config(load_config) {}

    void Load(FileResourceId resource_id, std::string &file_data);

    //iContextResourceTask
    void Execute(CommandQueueRef &output_queue) override;

private:
    iContextResourceLoader *const context_loader;

    math::ivec2 *const loaded_image_size;
    const Device::TextureHandle device_handle;
    const Configuration::TextureLoadConfig load_config;

    using ImageUniquePtr = std::unique_ptr<FIBITMAP, void (*)(FIBITMAP *)>;
    ImageUniquePtr loaded_image{nullptr, nullptr};

    void PreprocessImage(FREE_IMAGE_FORMAT fif);
};

} // namespace MoonGlare::Renderer::Resources::Texture
