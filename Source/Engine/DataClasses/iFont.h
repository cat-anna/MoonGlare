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

	virtual void GenerateCommands(Renderer::CommandQueue &Queue, uint16_t key) { }

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

	virtual FontInstance GenerateInstance(const wstring &text, const Descriptor *style = nullptr, 
										  bool UniformPosition = false) const = 0;

	virtual DataPath GetResourceType() const override final { return DataPath::Fonts; }
protected:
	virtual bool DoInitialize();
	virtual bool DoFinalize();
};

//-----------------------------------------------------------------------------

using FontClassRegister = Space::DynamicClassRegisterDeleter < iFont, ResourceFinalizer<iFont>, const string& > ;

} //namespace Fonts
} //namespace DataClasses 
} //namespace MoonGlare 

#endif // FONT_H
