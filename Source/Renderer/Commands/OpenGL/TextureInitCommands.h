#pragma once

#include "../../Resources/AssetLoaderInterface.h"
#include "../../Configuration.Renderer.h"
#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands {

namespace detail {


//void ApplyGlobalSettings() {
//	switch (::Settings->Graphic.Filtering) {
//	case Settings::FinteringMode::Bilinear:
//		SetBilinearFiltering();
//		break;
//	case Settings::FinteringMode::Trilinear:
//		SetTrilinearFiltering();
//		break;
//	case Settings::FinteringMode::Linear:
//		SetLinearFiltering();
//		break;
//	case Settings::FinteringMode::Nearest:
//		SetNearestFiltering();
//		break;
//	default:
//		AddLog(Error, "Unknown filtering mode!");
//		SetLinearFiltering();
//	}
//	SetClampToEdges();
//}

//void SetNearestFiltering(bool MipMapsEnabled = true) {
//	Bind();
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//}
//
//void SetLinearFiltering() {
//	Bind();
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//}
//
//void SetBilinearFiltering() {
//	Bind();
//	GenerateMipmaps();
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//}
//
//void SetTrilinearFiltering() {
//	Bind();
//	GenerateMipmaps();
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//}
//
//void SetClampToEdges() {
//	Bind();
//	glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	//glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//}
//void SetRepeatEdges() {
//	Bind();
//	glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	//glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_REPEAT);
//}

	struct Texture2DSetPixelsArgument;
}

//---------------------------------------------------------------------------------------


struct detail::Texture2DSetPixelsArgument {
	
	Resources::TextureLoader::TexturePixelData data;

	void Run() {
		auto &size = data.m_PixelSize;
		auto bpp = static_cast<unsigned>(data.m_PixelFormat);
		auto type = static_cast<unsigned>(data.m_PixelType);
		glTexImage2D(GL_TEXTURE_2D, /*MipmapLevel*/0, bpp, size[0], size[1], /*border*/0, bpp, type, data.m_Pixels);
		data.m_ImageMemory.reset();
	}
};

using Texture2DSetPixels = Commands::RunnableCommandTemplate<detail::Texture2DSetPixelsArgument>;

//---------------------------------------------------------------------------------------

}
