/*
  * Generated by cppsrc.sh
  * On 2017-02-04 20:57:16,59
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "../AssetManager.h"
#include "../FileSystem.h"
#include "TextureLoader.h"

#include "FreeImageUtils.h"

namespace MoonGlare::Asset::Texture {

Loader::Loader(FileSystem *fs):
	m_FileSystem(fs) {

	MoonGlareAssert(fs);
}

Loader::~Loader() {
}

//---------------------------------------------------------------------------------------

void Loader::Initialize() {
#ifndef _DISABLE_FREEIMAGE_LIB_
	FreeImage_SetOutputMessage(FreeImageErrorHandler);
#endif 
#ifdef DEBUG
	AddLog(System, "FreeImage version: " << FreeImage_GetVersion());
#endif
}

void Loader::Finalize() {
}

//---------------------------------------------------------------------------------------

bool Loader::LoadTexture(const std::string &fpath, TexturePixelData & out) {
	return LoadTextureURI(fpath, out);
}

//---------------------------------------------------------------------------------------

bool Loader::LoadTextureMemory(const void * ImgData, unsigned ImgLen, TexturePixelData &out) {
#ifdef _DISABLE_FREEIMAGE_LIB_
	return false;
#else
	FIMEMORY *fim = FreeImage_OpenMemory((BYTE*)ImgData, ImgLen);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);

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
	if (!dib) 
		return false;

	switch (fif) {
	case FIF_PNG: //png images need to be flipped
				  //	FreeImage_FlipVertical(dib);
		break;
	default:
		//nothing todo
		break;
	}

	//FreeImage_FlipVertical(dib);
	//FreeImage_FlipHorizontal(dib);
	switch (FreeImage_GetBPP(dib)) {
	case 32:
		SwapRedAndBlue(dib);
		out.m_PixelFormat = PixelFormat::RGBA8;
		break;
	case 24:
		if (fif != FIF_PNG)
			SwapRedAndBlue(dib);
		out.m_PixelFormat = PixelFormat::RGB8;
		break;
	default: {
		FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib);
		FreeImage_Unload(dib);
		dib = dib24;
		SwapRedAndBlue(dib);
		out.m_PixelFormat = PixelFormat::RGB8;
	}
	}

	out.m_PixelType = PixelType::UnsignedByte;
	out.m_Pixels = FreeImage_GetBits(dib);
	out.m_PixelSize = emath::usvec2(FreeImage_GetWidth(dib), FreeImage_GetHeight(dib));
	out.m_PixelsByteSize = out.m_PixelSize[0] * out.m_PixelSize[1] * (FreeImage_GetBPP(dib) / 8);

	out.m_ImageMemory = ImageUniquePtr((void*)dib, &DibDeallocator);
	return true;
#endif
}

bool Loader::LoadTextureURI(const std::string & URI, TexturePixelData & out) {
	StarVFS::ByteTable data;
	if (!GetFileSystem()->OpenFile(URI, DataPath::URI, data)) {
		//already logged, no need for more
		return false;
	}

	return LoadTextureMemory((char*)data.get(), data.byte_size(), out);
}

} //namespace MoonGlare::Asset::Texture
