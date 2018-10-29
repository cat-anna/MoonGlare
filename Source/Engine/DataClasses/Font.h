#pragma once 

#include <Renderer/PassthroughShaderDescriptor.h>

typedef struct FT_FaceRec_*  FT_Face;

namespace MoonGlare {
namespace DataClasses {
namespace Fonts {

struct FontGlyph;

class iFont {
public:
    using UniqueFontGlyph = std::unique_ptr<FontGlyph>;
    using FontGlyphMap = std::map<wchar_t, UniqueFontGlyph>;

    iFont(const string& URI);
    ~iFont();          

    DefineFlagGetter(m_Flags, Flags::Ready, Ready);
    DefineFlagGetter(m_Flags, Flags::Loaded, Loaded);

    bool Initialize();
    bool Finalize();

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

    FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const;
	
	bool RenderText(const std::wstring &text, Renderer::Frame *frame, const FontRenderRequest &options, const FontDeviceOptions &devopt, FontRect &outTextRect, FontResources &resources);
protected:
    struct Flags {
        enum {
            Ready  = 0x0001,
            Loaded = 0x0002,
        };
    };

    DefineFlagSetter(m_Flags, Flags::Ready, Ready);
    DefineFlagSetter(m_Flags, Flags::Loaded, Loaded);

    bool DoInitialize();
    bool DoFinalize();

	Renderer::ShaderResourceHandle<Renderer::PassthroughShaderDescriptor> m_ShaderHandle{ };
    bool GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame *frame, const std::wstring &text, const FontRenderRequest &options);
    FontGlyph* GetGlyph(wchar_t codepoint, Renderer::Commands::CommandQueue *q, Renderer::Frame * frame) const;
private:
    unsigned m_Flags = 0;
    std::string fileUri;
    StarVFS::ByteTable m_FontFile;
    FT_Face m_FontFace = nullptr;
    float m_CacheHight;

    mutable FontGlyphMap m_GlyphCache;
};

using Font = iFont;

//-----------------------------------------------------------------------------

} //namespace Fonts
} //namespace DataClasses 
} //namespace MoonGlare 
