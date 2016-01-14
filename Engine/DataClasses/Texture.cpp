/*
 * cTextureManager.cpp
 *
 *  Created on: 07-01-2014
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "FreeImageUtils.h"
#include <boost/filesystem.hpp>
#include <JobQueue.h>

namespace DataClasses {

GABI_IMPLEMENT_CLASS_SINGLETON(Texture)

Texture::Texture() : BaseClass() {
	SetThisAsInstance();
#ifndef _DISABLE_FREEIMAGE_LIB_
	FreeImage_SetOutputMessage(FreeImageErrorHandler);
#endif 
#ifdef DEBUG
	AddLog(SysInfo, "FreeImage version: " << FreeImage_GetVersion());
#endif
}

Texture::~Texture() {
}

//----------------------------------------------------------------------------------

Texture::SharedImage Texture::AllocateImage(const math::uvec2& size, BPP bpp) {
#ifndef _DISABLE_FREEIMAGE_LIB_
	auto si = std::make_shared<ImageInfo>();
	si->size = size;
	si->FIImage = (void*)FreeImage_Allocate(size[0], size[1], static_cast<int>(bpp));
	si->BPPtype = static_cast<int>(bpp);
	si->image = FreeImage_GetBits((FIBITMAP*)si->FIImage);
	return si;
#else
	return Texture::SharedImage();
#endif
}

void Texture::StoreImage(SharedImage image, const string &file, ImageFormat format) {
#ifndef _DISABLE_FREEIMAGE_LIB_
	if (image->value_type == Graphic::Flags::fBGR) {
		RGBToBGR((FIBITMAP*)image->FIImage);
		image->value_type = Graphic::Flags::fRGB;
	}

	if (image->value_type == Graphic::Flags::fBGRA) {
		RGBAToBGRA((FIBITMAP*)image->FIImage);
		image->value_type = Graphic::Flags::fRGBA;
	}

	FreeImage_Save(FIF_PNG, (FIBITMAP*)image->FIImage, file.c_str(), 0);
#endif
#if 0
		std::string tempf = Convert::ws2s(OutFile);
		using VisApp::Interface::iVisualizer;
		switch (format) {
		case iVisualizer::CaptureFileFormat::PNG:
			FreeImage_Save(FIF_PNG, (FIBITMAP*)image->FIImage, file.c_str(), 0);
			break;
		case iVisualizer::CaptureFileFormat::JPG:
			FreeImage_Save(FIF_JPEG, image, tempf.c_str(), 0);
			break;
		case iVisualizer::CaptureFileFormat::BMP:
		default:
			FreeImage_Save(FIF_BMP, image, tempf.c_str(), 0);
			break;
		}
#endif
}

void Texture::AsyncStoreImage(SharedImage image, string file, ImageFormat format) {
	JobQueue::QueueJob([image, file, format]() {
		Texture::StoreImage(image, file, format);
	});
}

//----------------------------------------------------------------------------------

bool Texture::LoadTexture(Graphic::Texture &tex, const char* data, unsigned datalen, bool ApplyDefaultSettings) {
	auto info = std::make_shared<ImageInfo>();
	if (!LoadImageMemory(data, datalen, *info.get())) {
		AddLog(Error, "Unable to load image memory!");
		return false;
	}
	tex.SetSize(info->size);
	Graphic::GetRenderDevice()->RequestContextManip([info, &tex, ApplyDefaultSettings]{
		tex.New();
		tex.SetTextureBits(info->image, info->size, info->BPPtype, info->value_type);
		if (ApplyDefaultSettings)
			tex.ApplyGlobalSettings();
		else
			tex.SetNearestFiltering();
		tex.BindEmpty();
	});
	return true;
}

bool Texture::LoadTexture(Graphic::Texture &tex, FileSystem::FileReader file, bool ApplyDefaultSettings) {
	if (!file)//silently ignore
		return false;
	return LoadTexture(tex, file->GetFileData(), file->Size(), ApplyDefaultSettings);
}

//----------------------------------------------------------------------------------


bool Texture::LoadImageMemory(const void* ImgData, unsigned ImgLen, ImageInfo& image) {
#ifndef _DISABLE_FREEIMAGE_LIB_
	FIBITMAP *dib = 0;
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
	dib = FreeImage_LoadFromMemory(fif, fim, flags);
	FreeImage_CloseMemory(fim);
	if(!dib) return false;

#if 0
	switch (fif) {
	case FIF_PNG: //png images need to be flipped
		//	FreeImage_FlipVertical(dib);
		break;
	default:
		//nothing todo
		break;
	}
#endif // 0

	//FreeImage_FlipVertical(dib);
	//FreeImage_FlipHorizontal(dib);
	switch (FreeImage_GetBPP(dib)){
	case 32:
		RGBAToBGRA(dib);
		image.BPPtype = Graphic::Flags::fRGBA; 
		break;
	case 24:
		RGBToBGR(dib);
		image.BPPtype = Graphic::Flags::fRGB;
		break; 
	default:{
		FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib);
		FreeImage_Unload(dib);
		dib = dib24;
		RGBToBGR(dib);
		image.BPPtype = Graphic::Flags::fRGB; 
	}
	}
	image.value_type = Graphic::Flags::fUnsignedChar;
	image.FIImage = dib;
	image.image = FreeImage_GetBits(dib);
	image.size[0] = FreeImage_GetWidth(dib);
	image.size[1] = FreeImage_GetHeight(dib);
#endif
	return true;
}

//----------------------------------------------------------------------------------

void Texture::ImageInfo::Unload() {
#ifndef _DISABLE_FREEIMAGE_LIB_
	if(!FIImage) return;
	FreeImage_Unload((FIBITMAP*)FIImage);
	FIImage = 0;
	image = 0;
#endif
}

} // namespace DataClasses
