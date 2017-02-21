#ifndef FONT_H
#define FONT_H

namespace MoonGlare {

namespace Renderer {
	struct CommandQueue;
}

namespace DataClasses {
namespace Fonts {

class Wrapper : public cRootClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(Wrapper, cRootClass);
	DISABLE_COPY();
public:
	Wrapper() { }
	~Wrapper() { }

	virtual void Render(Graphic::cRenderDevice &dev) = 0;
	virtual void RenderMesh(Graphic::cRenderDevice &dev) = 0;
	const math::vec2& GetSize() const { return m_size; }

	virtual void GenerateCommands(Renderer::Commands::CommandQueue &Queue, uint16_t key) { }

	bool AllowSubPixels() const { return m_AllowSubPixels; }
protected:
	math::vec2 m_size;
	bool m_AllowSubPixels;
};

class EmptyWrapper : public Wrapper {
	SPACERTTI_DECLARE_CLASS(EmptyWrapper, Wrapper);
	DISABLE_COPY();
public:
	EmptyWrapper() { }
	~EmptyWrapper() { }

	virtual void Render(Graphic::cRenderDevice &dev);
	virtual void RenderMesh(Graphic::cRenderDevice &dev);
protected:
};

//-----------------------------------------------------------------------------

class iFont : public DataClasses::DataClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iFont, DataClass);
	DISABLE_COPY();
public:
	iFont(const string& Name);
	virtual ~iFont();

	struct FontRect {
		math::fvec2 m_CanvasSize;
		math::fvec2 m_TextPosition;
		math::fvec2 m_TextBlockSize;
	};
	virtual FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const = 0;
	virtual FontInstance GenerateInstance(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const = 0;

	virtual DataPath GetResourceType() const override final { return DataPath::Fonts; }

	struct FontRenderRequest {
		float m_Size;
		emath::fvec4 m_Color;
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

	virtual bool RenderText(const std::wstring &text, Renderer::Frame *frame, const FontRenderRequest &options, FontRect &outTextRect, FontResources &resources) { return false; };
};

//-----------------------------------------------------------------------------

using FontClassRegister = Space::DynamicClassRegisterDeleter < iFont, ResourceFinalizer<iFont>, const string& > ;

} //namespace Fonts
} //namespace DataClasses 
} //namespace MoonGlare 

#endif // FONT_H
