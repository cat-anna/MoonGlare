#ifndef FONT_H
#define FONT_H

#include <Renderer/PassthroughShaderDescriptor.h>

namespace MoonGlare {
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

	virtual FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const = 0;
	
	bool RenderText(const std::wstring &text, Renderer::Frame *frame, const FontRenderRequest &options, const FontDeviceOptions &devopt, FontRect &outTextRect, FontResources &resources);
protected:
	Renderer::ShaderResourceHandle<Renderer::PassthroughShaderDescriptor> m_ShaderHandle{ };
	virtual bool GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame *frame, const std::wstring &text, const FontRenderRequest &options) = 0;
};

//-----------------------------------------------------------------------------

using FontClassRegister = Space::DynamicClassRegisterDeleter < iFont, ResourceFinalizer<iFont>, const string& > ;

} //namespace Fonts
} //namespace DataClasses 
} //namespace MoonGlare 

#endif // FONT_H
