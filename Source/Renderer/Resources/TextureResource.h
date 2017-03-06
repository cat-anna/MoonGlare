/*
  * Generated by cppsrc.sh
  * On 2017-02-13 22:45:35,93
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "../nfRenderer.h"
#include "AssetLoaderInterface.h"
#include "../Configuration.Renderer.h"

namespace MoonGlare::Renderer::Resources {

class alignas(16) TextureResource {
	using ThisClass = TextureResource;
	using Conf = Configuration::Texture;
	using ConfRes = Configuration::Resources;

public:
	void Initialize(ResourceManager* Owner, TextureLoader *TexLoader);
	void Finalize();

	bool Allocate(Commands::CommandQueue &queue, TextureResourceHandle &out);

	bool Allocate(TextureResourceHandle &out);
	bool Allocate(Frame *frame, TextureResourceHandle &out);
	void Release(Frame *frame, TextureResourceHandle h);

	bool LoadTexture(TextureResourceHandle &out, const std::string &fPath, 
		Configuration::TextureLoad config = Configuration::TextureLoad::Default(),
		bool CanAllocate = true);

	TextureHandle* GetHandleArrayBase() { return &m_GLHandle[0]; }

	emath::usvec2 GetSize(TextureResourceHandle h) const;
private: 
	template<typename T>
	using Array = std::array<T, Conf::Limit>;
	using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

	Bitmap m_AllocationBitmap;
	Array<TextureHandle> m_GLHandle;
	//Array<Asset::FileHash> m_SourceHash;
	Array<emath::usvec2> m_TextureSize;
	ResourceManager *m_ResourceManager = nullptr;
	TextureLoader *m_TexureLoader = nullptr;
	const Configuration::Texture *m_Settings = nullptr;

	DeclarePerformanceCounter(SuccessfulAllocations);
	DeclarePerformanceCounter(SuccessfulDellocations);
	DeclarePerformanceCounter(FailedAllocations);
	DeclarePerformanceCounter(FailedDellocations);
	DeclarePerformanceCounter(OpenGLAllocations);
	DeclarePerformanceCounter(OpenGLDeallocations);
};

static_assert((sizeof(TextureResource) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<TextureResource>::value, "Invalid size!");//atomics

} //namespace MoonGlare::Renderer::Resources 
