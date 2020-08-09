#pragma once

#include <Foundation/iFileSystem.h>

#include "../../nfRenderer.h"
#include "../../Configuration.Renderer.h"
#include "../../Commands/CommandQueue.h"

namespace MoonGlare::Renderer::Resources {

class 
    //alignas(16) 
    TextureResource {
	using ThisClass = TextureResource;
	using Conf = Configuration::Texture;
	using ConfRes = Configuration::Resources;

public:
	void Initialize(ResourceManager* Owner, iFileSystem *fileSystem);
	void Finalize();

	bool Allocate(TextureResourceHandle &out);
    TextureResourceHandle Allocate() {
        TextureResourceHandle r = {};
        Allocate(r);
        return r;
    }
	void Release(TextureResourceHandle h);

    TextureResourceHandle AllocExtTexture(const std::string &uri, Device::TextureHandle devTex, const emath::usvec2 &size = { 0,0 });

    bool LoadTexture(TextureResourceHandle &hout, const std::string &uri,
        Configuration::TextureLoad config = Configuration::TextureLoad::Default(),
        bool CanAllocate = true);

    TextureResourceHandle LoadTexture(const std::string &uri, Configuration::TextureLoad config = Configuration::TextureLoad::Default()) {
        TextureResourceHandle h;
        LoadTexture(h, uri, config);
        return h;
    }

	template<typename T>
	bool SetTexturePixels(TextureResourceHandle &out, Commands::CommandQueue &q, const T* Pixels, const emath::usvec2 &size,
		Configuration::TextureLoad loadcfg, Device::PixelFormat internalformat, Device::PixelFormat format, bool AllowAllocate = true, Commands::CommandKey key = Commands::CommandKey()) {
		return SetTexturePixels(out, q, Pixels, size, loadcfg, internalformat, format, AllowAllocate, (Device::ValueFormat)Device::TypeId<T>, key);
	}

    bool SetTexturePixels(TextureResourceHandle &out, Commands::CommandQueue &q, const void* Pixels, const emath::usvec2 &size,
        Configuration::TextureLoad config, Device::PixelFormat internalformat, Device::PixelFormat format, bool AllowAllocate, Device::ValueFormat TypeValue, Commands::CommandKey key = Commands::CommandKey());

    TextureResourceHandle CreateTexture(Commands::CommandQueue &q, const void* Pixels, const emath::usvec2 &size,
        Configuration::TextureLoad config, Device::PixelFormat internalformat, Device::PixelFormat format, Device::ValueFormat TypeValue, Commands::CommandKey key = Commands::CommandKey()) {
        TextureResourceHandle h;
        SetTexturePixels(h, q, Pixels, size, config, internalformat, format, true, TypeValue, key);
        return h;
    }

	Device::TextureHandle* GetHandleArrayBase() { return &m_GLHandle[0]; }

	emath::usvec2 GetSize(TextureResourceHandle h) const;
    bool IsHandleValid(TextureResourceHandle h) const;
private: 
	template<typename T>
	using Array = std::array<T, Conf::Limit>;
	using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    Array<TextureResourceHandle::Generation_t> generations;
    Bitmap m_AllocationBitmap;
	Array<Device::TextureHandle> m_GLHandle;
	Array<emath::usvec2> m_TextureSize;

    std::unordered_map<std::string, TextureResourceHandle> loadedTextures; //temporary solution

	ResourceManager *m_ResourceManager = nullptr;
	const Configuration::Texture *m_Settings = nullptr;
};

//static_assert((sizeof(TextureResource) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<TextureResource>::value, "Invalid size!");//atomics

} //namespace MoonGlare::Renderer::Resources 
