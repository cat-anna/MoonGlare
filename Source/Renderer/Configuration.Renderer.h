#pragma once

namespace MoonGlare::Renderer::Configuration {

struct Requirement {
	//ogl
	static constexpr uint32_t OpenGLVersionMajor = 4;
	static constexpr uint32_t OpenGLVersionMinor = 2;
	
	//system 
	static constexpr uint32_t CPUCores = 2;
};

struct FrameResourceStorage {
	static constexpr uint32_t TextureLimit = 64;
	static constexpr uint32_t VAOLimit = 64;
};

struct FrameBuffer {
	static constexpr uint32_t MemorySize = 1 * 32 * 1024;
	static constexpr uint32_t Count = 3;
	static constexpr uint32_t SubQueueCount = 64;

	enum class Layer {
		Controll,
		PreRender, //TextureRenderTask and friends
		//ShadowMaps,
		//Render,
		//DefferedGeometry,
		//DefferedLighting,
		//PostRender,
		//Postprocess,

		MaxValue,
	};
};

struct Context {
	enum class Window {
		First,
		//Second,
		//Third,
		//Fourth,

		MaxValue,
	};
};

struct CommandBucket {
	static constexpr uint32_t MaxCommandsPerBucket = 16;
	using BucketCommandCount = uint8_t;
	using BucketCommandIndex = uint16_t;
	static constexpr uint32_t MaxBuckets = 1024;
	static constexpr uint32_t MaxCommands = 4096;

	static constexpr uint32_t ArgumentMemoryBuffer = 1 * 128 * 1024;
};

struct CommandQueue {
	static constexpr uint32_t ArgumentMemoryBuffer = 1 * 64 * 1024; 
	static constexpr uint32_t CommandLimit = 4096;
	static constexpr uint32_t BytesPerCommand = ArgumentMemoryBuffer / CommandLimit;
};

struct IndexBuffer {
	static constexpr uint32_t TextureBuffer = 256;
};
struct Resources {
	template<size_t SIZE>
	using BitmapAllocator = ::Space::Memory::LinearAtomicBitmapAllocator<SIZE, uint32_t, uint32_t>;
};

//---------------------------------------------------------------------------------------

struct Texture {
	static constexpr uint32_t Limit = 1024;
	static constexpr uint32_t Initial = 64;

	enum class Filtering {
		Nearest,
		Linear,
		Bilinear,
		Trilinear,

		MaxValue,
		Default = MaxValue,
	};

	enum class Edges {
		Repeat,
		Clamp,
		MaxValue,
		Default = MaxValue,
	};

	Filtering m_Filtering;

	void ResetToDefault() {
		m_Filtering = Filtering::Bilinear;
	}
};

struct TextureLoad {
	using Conf = Texture;
	Conf::Filtering m_Filtering;
	Conf::Edges m_Edges;

	static TextureLoad Default() {
		return {
			Conf::Filtering::Default,
			Conf::Edges::Default,
		};
	}
};

struct VAO {
	static constexpr uint32_t VAOLimit = 1024;
	static constexpr uint32_t VAOInitial = 64;
	static constexpr uint32_t MaxBuffersPerVAO = 8;

	using VAOBuffers = std::array<BufferHandle, MaxBuffersPerVAO>;

	using ChannelType = GLuint;

	enum class InputChannels : ChannelType {
		Vertex,
		Texture0,
		Normals,

		//Color, //unused

		Index = 4,

		MaxValue,
	};

	static_assert(static_cast<uint32_t>(InputChannels::MaxValue) <= MaxBuffersPerVAO, "mismatched buffer count!");
};

struct Shader {
	static constexpr uint32_t Limit = 32;
	static constexpr uint32_t UniformLimit = 16;
	using UniformLocations = std::array<ShaderUniformHandle, UniformLimit>;
};

struct TextureRenderTask {
	static constexpr uint32_t Limit = 64;
};

struct Material {
	static constexpr uint32_t Limit = 1024;

};

//---------------------------------------------------------------------------------------

struct RuntimeConfiguration {
	Texture m_Texture;

	void ResetToDefault() {
		m_Texture.ResetToDefault();
	}
};

} //namespace MoonGlare::Renderer::Configuration
