#pragma once 

#include <Renderer/PassthroughShaderDescriptor.h>

typedef struct FT_FaceRec_*  FT_Face;

namespace MoonGlare {
namespace DataClasses {
namespace Fonts {

class iFont {
public:
    iFont(const string& URI);
    ~iFont();       

    static constexpr uint32_t FontFacesPerDim = 16;
    static constexpr float DeltaTexUV = 1.0f / (float)FontFacesPerDim;

private:
    struct FontGlyph {
        emath::fvec2 fontFacePosition;
        math::vec2 m_Advance;
        math::vec2 m_Position;
        math::vec2 charSize;
    };

    using FontGlyphMap = std::unordered_map<wchar_t, FontGlyph>;

    std::unique_ptr<uint8_t> facesTexture;
    Renderer::MaterialResourceHandle facesMaterial = { };
    uint8_t faceTextureSize = 0;
    mutable bool faceTextureDirty = false;
    mutable uint32_t faceAllocIndex = 1;
    mutable FontGlyphMap glyphCache;

    std::string fileUri;
    StarVFS::ByteTable fontFileMemory;


    uint8_t* GetTextureScanLine(emath::fvec2 pos, uint32_t line) const {
        uint32_t lineSize = faceTextureSize * FontFacesPerDim;
        size_t v = (size_t)pos.y() * faceTextureSize;
        return facesTexture.get() 
            + (v + line) * lineSize
            + (size_t)pos.x() * faceTextureSize;
    }

    void ReloadFacesTexture(MoonGlare::Renderer::Frame * frame, MoonGlare::Renderer::Commands::CommandQueue & q);

    void DumpFacesTexture();
    void DumpFontCodePoints();

    //old
public:
    using UniqueFontGlyph = std::unique_ptr<FontGlyph>;

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

	Renderer::ShaderResourceHandle<Renderer::PassthroughShaderDescriptor> m_ShaderHandle{ };
    bool GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame *frame, const std::wstring &text, const FontRenderRequest &options);
    FontGlyph* GetGlyph(wchar_t codepoint) const;

    unsigned m_Flags = 0;
    FT_Face m_FontFace = nullptr;
    float m_CacheHight;

};

using Font = iFont;

//-----------------------------------------------------------------------------

} //namespace Fonts
} //namespace DataClasses 
} //namespace MoonGlare 
