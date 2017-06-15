#pragma once

#include "../../Resources/AssetLoaderInterface.h"
#include "../../Configuration.Renderer.h"
#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands {

namespace detail {
struct Texture2DSetPixelDataArgument;
struct Texture2DSetPixelsArrayArgument;
}

//---------------------------------------------------------------------------------------

struct detail::Texture2DSetPixelDataArgument {
	Resources::TextureLoader::TexturePixelData data;

	void Run() {
		auto &size = data.m_PixelSize;
		auto bpp = static_cast<unsigned>(data.m_PixelFormat);
		auto type = static_cast<unsigned>(data.m_PixelType);
		glTexImage2D(GL_TEXTURE_2D, /*MipmapLevel*/0, bpp, size[0], size[1], /*border*/0, bpp, type, data.m_Pixels);
		data.m_ImageMemory.reset();
	}
};
using Texture2DSetPixelData = Commands::RunnableCommandTemplate<detail::Texture2DSetPixelDataArgument>;

struct detail::Texture2DSetPixelsArrayArgument {
	unsigned short m_Size[2];
	GLenum m_BPP;
	GLenum m_Type;
	const void *m_PixelData;
		 
	void Run() {
		glTexImage2D(GL_TEXTURE_2D, /*MipmapLevel*/0, m_BPP, m_Size[0], m_Size[1], /*border*/0, m_BPP, m_Type, m_PixelData);
	}
};
using Texture2DSetPixelsArray = Commands::RunnableCommandTemplate<detail::Texture2DSetPixelsArrayArgument>;

//---------------------------------------------------------------------------------------

}
