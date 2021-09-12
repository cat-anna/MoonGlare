#pragma once

#include "command_queue.hpp"
#include "math/vector.hpp"
#include "memory/element_pool.hpp"
#include "renderer/configuration/texture.hpp"
#include "renderer/device_types.hpp"
#include "renderer/resources/texture_resource_interface.hpp"
#include "texture_loader_interface.hpp"
#include <gsl/gsl>
#include <string>
#include <unordered_map>

namespace MoonGlare::Renderer::Resources::Texture {

class TextureResource final : public iTextureResource {
public:
    TextureResource(gsl::not_null<iTextureResourceLoader *> loader_interface,
                    gsl::not_null<CommandQueue *> init_commands);
    ~TextureResource() override;

    using TextureResourceHandlePool =
        Memory::HandleElementPool<Device::TextureHandle, Configuration::Texture::kStandbyPoolSize,
                                  GLint, Device::kInvalidTextureHandle>;

    void ReleaseResources(CommandQueue *command_queue); //TODO

    // iTextureResource
    ResourceHandle LoadTextureResource(FileResourceId file_id) override;
    TextureHandle LoadTexture(const std::string &name) override;

    // TextureResourceHandle AllocExtTexture(const std::string &uri, Device::TextureHandle devTex,
    //                                       const emath::usvec2 &size = {0, 0});

    // bool LoadTexture(TextureResourceHandle &hout, const std::string &uri,
    //                  Configuration::TextureLoad config = Configuration::TextureLoad::Default(),
    //                  bool CanAllocate = true);

    // TextureResourceHandle
    // LoadTexture(const std::string &uri,
    //             Configuration::TextureLoad config = Configuration::TextureLoad::Default()) {
    //     TextureResourceHandle h;
    //     LoadTexture(h, uri, config);
    //     return h;
    // }

    // template <typename T>
    // bool SetTexturePixels(TextureResourceHandle &out, Commands::CommandQueue &q, const T *Pixels,
    //                       const emath::usvec2 &size, Configuration::TextureLoad loadcfg,
    //                       Device::PixelFormat internalformat, Device::PixelFormat format,
    //                       bool AllowAllocate = true,
    //                       Commands::CommandKey key = Commands::CommandKey()) {
    //     return SetTexturePixels(out, q, Pixels, size, loadcfg, internalformat, format,
    //                             AllowAllocate, (Device::ValueFormat)Device::TypeId<T>, key);
    // }

    // bool SetTexturePixels(TextureResourceHandle &out, Commands::CommandQueue &q, const void *Pixels,
    //                       const emath::usvec2 &size, Configuration::TextureLoad config,
    //                       Device::PixelFormat internalformat, Device::PixelFormat format,
    //                       bool AllowAllocate, Device::ValueFormat TypeValue,
    //                       Commands::CommandKey key = Commands::CommandKey());

    // TextureResourceHandle CreateTexture(Commands::CommandQueue &q, const void *Pixels,
    //                                     const emath::usvec2 &size,
    //                                     Configuration::TextureLoad config,
    //                                     Device::PixelFormat internalformat,
    //                                     Device::PixelFormat format, Device::ValueFormat TypeValue,
    //                                     Commands::CommandKey key = Commands::CommandKey()) {
    //     TextureResourceHandle h;
    //     SetTexturePixels(h, q, Pixels, size, config, internalformat, format, true, TypeValue, key);
    //     return h;
    // }

    // Device::TextureHandle *GetHandleArrayBase() { return &m_GLHandle[0]; }

    // emath::usvec2 GetSize(TextureResourceHandle h) const;
    // bool IsHandleValid(TextureResourceHandle h) const;

private:
    iTextureResourceLoader *const loader_interface;

    TextureResourceHandlePool standby_handle_pool;

    template <typename T>
    using DataArray = std::array<T, Configuration::Texture::kLimit>;

    DataArray<FileResourceId> loaded_resource_id;

    // using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;
    // Array<TextureResourceHandle::Generation_t> generations;
    // Bitmap m_AllocationBitmap;
    // Array<Device::TextureHandle> m_GLHandle;
    DataArray<math::ivec2> texture_size;

    std::unordered_map<FileResourceId, Device::TextureHandle> loaded_textures;

    // ResourceManager *m_ResourceManager = nullptr;
    // const Configuration::Texture *m_Settings = nullptr;
};

//static_assert((sizeof(TextureResource) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<TextureResource>::value, "Invalid size!");//atomics

} // namespace MoonGlare::Renderer::Resources::Texture
