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

namespace MoonGlare {
namespace DataClasses {

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
		SwapRedAndBlue((FIBITMAP*)image->FIImage);
		image->value_type = Graphic::Flags::fRGB;
	}

	if (image->value_type == Graphic::Flags::fBGRA) {
		SwapRedAndBlue((FIBITMAP*)image->FIImage);
		image->value_type = Graphic::Flags::fRGBA;
	}

	FreeImage_Save(FIF_PNG, (FIBITMAP*)image->FIImage, file.c_str(), 0);
#endif
}

void Texture::AsyncStoreImage(SharedImage image, string file, ImageFormat format) {
	JobQueue::QueueJob([image, file, format]() {
		Texture::StoreImage(image, file, format);
	});
}

void Texture::ImageInfo::Unload() {
#ifndef _DISABLE_FREEIMAGE_LIB_
	if(!FIImage) return;
	FreeImage_Unload((FIBITMAP*)FIImage);
	FIImage = 0;
	image = 0;
#endif
}

} // namespace DataClasses
} //namespace MoonGlare 
