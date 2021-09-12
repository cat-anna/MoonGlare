
#include "free_image_loader.hpp"
#include "command_queue.hpp"
#include "commands/texture_load_commands.hpp"
#include "debugger_support.hpp"
#include "free_image_utils.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Renderer::Resources::Texture {

void FreeImageLoader::ScheduleLoad(FileResourceId resource_id, Device::TextureHandle device_handle,
                                   math::ivec2 *loaded_image_size,
                                   Configuration::TextureLoadConfig load_config) {
    load_config.Check(*global_config);
    auto req = std::make_shared<FreeImageLoaderRequest>(context_loader, device_handle,
                                                        loaded_image_size, load_config);

    async_loader->LoadFile(
        resource_id, [req = std::move(req), ctx_loader = context_loader,
                      device_handle](FileResourceId resource_id, std::string &file_data) {
            req->Load(resource_id, file_data);
            ctx_loader->PushResourceTask(req);
            AddLog(Resources, fmt::format("Loaded texture {}->{}", resource_id, device_handle));
        });
}

void FreeImageLoaderRequest::Load(FileResourceId resource_id, std::string &file_data) {
    FIMEMORY *fim = FreeImage_OpenMemory((BYTE *)file_data.data(), file_data.size());
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);

    AddLog(Resources, fmt::format("Detected texture format {}->{}", resource_id, fif));

    int flags = 0;
    switch (fif) {
    case FIF_PNG:
        //flags |= PNG_IGNOREGAMMA;
        break;
    default:
        break;
    }

    FIBITMAP *dib = FreeImage_LoadFromMemory(fif, fim, flags);
    FreeImage_CloseMemory(fim);

    if (loaded_image_size != nullptr) {
        *loaded_image_size = {FreeImage_GetWidth(dib), FreeImage_GetHeight(dib)};
    }

    loaded_image = ImageUniquePtr(dib, &DibDeallocator);

    PreprocessImage(fif);
}

void FreeImageLoaderRequest::Execute(CommandQueueRef &output_queue) {
    auto bitmap = loaded_image.get();

    Device::PixelFormat format = Device::PixelFormat::RGB8;
    if (FreeImage_GetBPP(bitmap) == 32) {
        format = Device::PixelFormat::RGBA8;
    }

    output_queue->PushCommand(Commands::TextureLoadCommandCommand{
        .texture_handle = device_handle,
        .pixels = FreeImage_GetBits(bitmap),
        .size = {static_cast<GLsizei>(FreeImage_GetWidth(bitmap)),
                 static_cast<GLsizei>(FreeImage_GetHeight(bitmap))},
        .value_format = Device::ValueFormat::UnsignedByte,
        .pixel_format = format,
        .config = load_config,
    });
}

void FreeImageLoaderRequest::PreprocessImage(FREE_IMAGE_FORMAT fif) {
    auto bitmap = loaded_image.get();

    // Device::PixelFormat srcFormat = Device::PixelFormat::RGB8;
    Device::PixelFormat format = Device::PixelFormat::RGB8;

    auto mask = FreeImage_GetBlueMask(bitmap);
    // //config.m_Flags.useSRGBColorSpace = false;

    switch (FreeImage_GetBPP(bitmap)) {
    case 32:
        if (mask != FI_RGBA_RED_MASK) {
            SwapRedAndBlue(bitmap);
        }
        format = Device::PixelFormat::RGBA8;
        // srcFormat = config.m_Flags.useSRGBColorSpace ? PixelFormat::SRGBA8 : PixelFormat::RGBA8;
        break;
    case 24:
        if (mask != FI_RGBA_RED_MASK) {
            SwapRedAndBlue(bitmap);
        }
        format = Device::PixelFormat::RGB8;
        // srcFormat = config.m_Flags.useSRGBColorSpace ? PixelFormat::SRGB8 : PixelFormat::RGB8;
        break;
    default: {
        TriggerBreakPoint();
        FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(bitmap);
        FreeImage_Unload(loaded_image.release());
        loaded_image.reset(bitmap = dib24);
        SwapRedAndBlue(bitmap);
        format = Device::PixelFormat::RGB8;
    }
    }
}

} // namespace MoonGlare::Renderer::Resources::Texture
