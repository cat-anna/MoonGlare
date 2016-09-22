#ifndef C2DTEXTURE_H_
#define C2DTEXTURE_H_

namespace Graphic {

template <GLenum TEX_MODE>
class Texture_template {
protected:
	unsigned m_Handle;
	unsigned m_Flags;
	math::uvec2 m_Size;
public:
	Texture_template() : m_Handle(0), m_Flags(0) { }
	Texture_template(const Texture_template<TEX_MODE> &other) = delete;
	~Texture_template() { 
		if (!(m_Flags & Flag_SlaveMode))
			Free(); 
	}
	Texture_template<TEX_MODE> &operator=(const Texture_template<TEX_MODE>& other) {
		m_Handle = other.m_Handle;
		m_Flags = other.m_Flags | Flag_SlaveMode;
		return *this;
	}

	void swap(Texture_template<TEX_MODE> &oth) {
		std::swap(m_Handle, oth.m_Handle);
		std::swap(m_Flags, oth.m_Flags);
		std::swap(m_Size, oth.m_Size);
	}

	unsigned Handle() const { return m_Handle; }

	const math::uvec2& GetSize() const { return m_Size; }
	void SetSize(const math::uvec2 &size) { m_Size = size; }

	typedef Texture_template<TEX_MODE> ThisClass;

	const ThisClass* Bind(unsigned TextureChannel)  const {
		glActiveTexture(GL_TEXTURE0 + TextureChannel);
		glBindTexture(TEX_MODE, m_Handle);
		return this;
	}
	const ThisClass* Bind()  const {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(TEX_MODE, m_Handle);
		return this;
	}

	static void BindEmpty(unsigned TextureChannel = 0) {
		glActiveTexture(GL_TEXTURE0 + TextureChannel);
		glBindTexture(TEX_MODE, 0);
	}

	static void ActivateChannel(unsigned channel) {
		glActiveTexture(GL_TEXTURE0 + channel);
	}

	void Free() {
		if (m_Flags & Flag_SlaveMode) return;
		if (!m_Handle) return;
		glDeleteTextures(1, &m_Handle);
		m_Handle = 0;
	}

	void New() {
		if (m_Flags & Flag_SlaveMode) return;
		Free();//just in case
		glGenTextures(1, &m_Handle);
		Bind();
	}

	void SetTextureBits(void* data, const math::uvec2 &size, unsigned bpp, unsigned type, unsigned MipmapLevel = 0) {
		New();
		m_Size = size;
		glTexImage2D(TEX_MODE, MipmapLevel, bpp, size[0], size[1], 0, bpp, type, data);
	}

	void SetNearestFiltering(bool MipMapsEnabled = true) {
		Bind();
		glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	void SetLinearFiltering() {
		Bind();
		glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	void SetBilinearFiltering() {
		Bind();
		GenerateMipmaps();
		glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}

	void SetTrilinearFiltering() {
		Bind();
		GenerateMipmaps();
		glTexParameteri(TEX_MODE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(TEX_MODE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	void SetClampToEdges() {
		Bind();
		glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	void SetRepeatEdges() {
		Bind();
		glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(TEX_MODE, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}
	
	enum {
		Flag_SlaveMode		= 0x1000,
	};

	void ApplyGlobalSettings() {
		switch (::Settings->Graphic.Filtering) {
		case Settings::FinteringMode::Bilinear:
			SetBilinearFiltering();
			break;
		case Settings::FinteringMode::Trilinear:
			SetTrilinearFiltering();
			break;
		case Settings::FinteringMode::Linear:
			SetLinearFiltering();
			break;
		case Settings::FinteringMode::Nearest:
			SetNearestFiltering();
			break;
		default:
			AddLog(Error, "Unknown filtering mode!");
			SetLinearFiltering();
		}
		SetClampToEdges();
	}
protected:
	void GenerateMipmaps() { glGenerateMipmap(TEX_MODE); }
};

} // namespace Graphic

#endif // C2DTEXTURE_H_ 
