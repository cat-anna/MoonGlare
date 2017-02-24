#ifndef FONT_H
#define FONT_H

namespace MoonGlare {

namespace Renderer {
	struct CommandQueue;
}

namespace DataClasses {
namespace Fonts {

class iFont : public DataClasses::DataClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iFont, DataClass);
public:
	iFont(const string& Name) : BaseClass(Name) { }

	struct FontRect {
		math::fvec2 m_CanvasSize;
		math::fvec2 m_TextPosition;
		math::fvec2 m_TextBlockSize;
	};
	virtual FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const = 0;

	virtual DataPath GetResourceType() const override final { return DataPath::Fonts; }

	struct FontRenderRequest {
		float m_Size;
		emath::fvec4 m_Color;
	};

	struct FontDeviceOptions {
		bool m_UseUniformMode;
		emath::ivec2 m_DeviceSize;
	};

	struct FontResources {
		Renderer::TextureResourceHandle m_Texture;
		Renderer::VAOResourceHandle m_VAO;

		template<typename T>
		void Release(T *t) {
			t->typename ReleaseResource(m_Texture);
			t->typename ReleaseResource(m_VAO);
		}
	};

	virtual bool RenderText(const std::wstring &text, Renderer::Frame *frame, const FontRenderRequest &options, const FontDeviceOptions &devopt, FontRect &outTextRect, FontResources &resources) { return false; };
};

//-----------------------------------------------------------------------------

using FontClassRegister = Space::DynamicClassRegisterDeleter < iFont, ResourceFinalizer<iFont>, const string& > ;

} //namespace Fonts
} //namespace DataClasses 
} //namespace MoonGlare 

#endif // FONT_H
