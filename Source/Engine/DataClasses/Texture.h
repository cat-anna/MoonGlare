/*
 * cTextureManager.h
 *
 *  Created on: 07-01-2014
 *      Author: Paweu
 */
#ifndef CTEXTUREMANAGER_H_
#define CTEXTUREMANAGER_H_

namespace MoonGlare {
namespace DataClasses {

class Texture : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(Texture, cRootClass)
public:
	Texture();
	~Texture();

	enum class BPP {
		RGB = 24,
		RGBA = 32,
	};

	enum class ImageFormat {
		PNG,
	};

	struct ImageInfo {
		ImageInfo() : size(0, 0), BPPtype(0), image(0), FIImage(0) { };
		~ImageInfo() { Unload(); }
		unsigned BPPtype, value_type;
		math::uvec2 size;
		unsigned char* image;
		void *FIImage;
		void Unload();
	};

	using SharedImage = std::shared_ptr < ImageInfo > ;

	static SharedImage AllocateImage(const math::uvec2& size, BPP bpp);

	static void StoreImage(SharedImage image, const string &file, ImageFormat format = ImageFormat::PNG);
	static void AsyncStoreImage(SharedImage image, string file, ImageFormat format = ImageFormat::PNG);

	static bool LoadTexture(Graphic::Texture &tex, const char* data, unsigned datalen, bool ApplyDefaultSettings = true);
protected:
	static bool LoadImageMemory(const void* ImgData, unsigned ImgLen, ImageInfo& image);
};

} // namespace DataClasses
} //namespace MoonGlare 

#endif // CTEXTUREMANAGER_H_ 
