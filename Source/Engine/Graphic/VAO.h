#pragma once
#ifndef VAO_H
#define VAO_H

namespace Graphic { 
 
class VAO {
public:
	struct MeshData {
		unsigned __int32 BaseVertex;
		unsigned __int32 BaseIndex;
		unsigned __int32 NumIndices;
		unsigned __int32 ElementMode;

		MeshData() { BaseVertex = BaseIndex = NumIndices = 0; }
		~MeshData() { }
	};

	class MeshVector : public std::vector<MeshData> {
	public:
		void push_back_mesh(unsigned __int32 BaseVertex, unsigned __int32 BaseIndex, unsigned __int32 NumIndices, unsigned __int32 ElementMode) {
			push_back(MeshData());
			MeshData &md = back();
			md.BaseVertex = BaseVertex;
			md.BaseIndex = BaseIndex;
			md.NumIndices = NumIndices;
			md.ElementMode = ElementMode;
		}
	};

	enum {
		CoordChannel,
		UVChanel,
		NormalChannel,
		ColorChannel,

		IndexChannel,
		MaxChannel = 8,
	};

	VAO() : m_VAO(0) { memset(m_Buffers, 0, sizeof(m_Buffers)); }
	~VAO() { if(m_VAO) Finalize(); }

	/** This function SWAPS vectors content into internal storage. */
	void DelayInit(VertexVector &Verticles, TexCoordVector &TexCoords, NormalVector &Normals, IndexVector &Index) { Initialize(Verticles, TexCoords, Normals, Index); }
	/** This function SWAPS vectors content into internal storage. */
	void Initialize(VertexVector &Verticles, TexCoordVector &TexCoords, NormalVector &Normals, IndexVector &Index);
	void Finalize();

	void Bind() const { glBindVertexArray(m_VAO); }
	void UnBind() const { glBindVertexArray(0); }

	void RenderMesh(const MeshVector &input) const {
		Bind();
		for (auto &mesh : input) {
			DrawElements(mesh.NumIndices, mesh.BaseIndex, mesh.BaseVertex, mesh.ElementMode);
		}
		UnBind();
	}
	
	void DrawElements(const MeshData& mesh) const {
		DrawElements(mesh.NumIndices, mesh.BaseIndex, mesh.BaseVertex, mesh.ElementMode);
	}

	void DrawElements(unsigned NumIndices, unsigned BaseIndex, unsigned BaseVertex, unsigned ElementsType) const {
 	     glDrawElementsBaseVertex(ElementsType, NumIndices, m_IndexValueType, (void*)(m_IndexTypeSize * BaseIndex), BaseVertex);
 //	     glDrawElements(ElementsType, NumIndices, m_IndexValueType, (void*)(m_IndexTypeSize * BaseIndex));
	}

	void SetDataChannel(GLuint Channel, const void* data, size_t ElementCount, const Types::TypeInfo& typeinfo, bool Dynamic = false) {
		glBindBuffer(GL_ARRAY_BUFFER, GetChannel(Channel));
		glBufferData(GL_ARRAY_BUFFER, typeinfo.ElementSize * ElementCount * typeinfo.TypeSize, data, Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		glEnableVertexAttribArray(Channel);
		glVertexAttribPointer(Channel, typeinfo.ElementSize, typeinfo.GLTypeValue, GL_FALSE, 0, 0);
	}

	void WriteIndexes(const void* data, size_t Count, const Types::TypeInfo& typeinfo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GetChannel(IndexChannel));
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Count * typeinfo.TypeSize, data, GL_STATIC_DRAW);
		m_IndexValueType = typeinfo.GLTypeValue;
		m_IndexTypeSize = typeinfo.TypeSize;
	}

	template <unsigned LEN, class T>
	void SetDataChannel(GLuint Channel, const T* data, size_t Count, bool Dynamic = false) {
		Types::TypeInfo t(LEN, sizeof(T), Types::glType<T>::value());
		SetDataChannel(Channel, data, Count, t, Dynamic);
	}

	template <class T>
	void WriteIndexes(const T* data, size_t Count) {
		Types::TypeInfo t(1, sizeof(T), Types::glType<T>::value());
		WriteIndexes(data, Count, t);
	}

	void swap(VAO &dest) {
		for (size_t i = 0; i < MaxChannel; ++i)
			std::swap(m_Buffers[i], dest.m_Buffers[i]);
		std::swap(m_VAO, dest.m_VAO);
		std::swap(m_IndexValueType, dest.m_IndexValueType);
		std::swap(m_IndexTypeSize, dest.m_IndexTypeSize);
	}

	void New() {
		if (m_VAO) Free();
		glGenVertexArrays(1, &m_VAO);
	}
	GLuint Handle() const { return m_VAO; }
	GLuint IndexValueType() const { return m_IndexValueType; }
	GLuint ChannelHandle(GLuint Channel) const { return m_Buffers[Channel]; }
protected:
	GLuint GetChannel(GLuint Channel) {
		ASSERT(Channel < MaxChannel);
		if (!m_Buffers[Channel])
			glGenBuffers(1, m_Buffers + Channel);
		return m_Buffers[Channel];
	}

	void Free() {
		if(m_VAO){
			glDeleteBuffers(MaxChannel, m_Buffers);
			glDeleteVertexArrays(1, &m_VAO);
		}
		m_VAO = 0;
	}

private:
	GLuint m_Buffers[8];
	GLuint m_VAO;

	GLuint m_IndexValueType = 0;
	GLuint m_IndexTypeSize = 0;
};

} //namespace Graphic

#endif
