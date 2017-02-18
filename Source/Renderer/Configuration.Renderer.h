#pragma once

namespace MoonGlare::Renderer::Configuration {

struct Requirement {
	//ogl
	static constexpr uint32_t OpenGLVersionMajor = 4;
	static constexpr uint32_t OpenGLVersionMinor = 2;
	
	//system 
	static constexpr uint32_t CPUCores = 2;
};

struct FrameBuffer {
	static constexpr uint32_t MemorySize = 1 * 32 * 1024;
	static constexpr uint32_t Count = 3;

	enum class Layers {
		Controll,

		PreRender, //TextureRenderTask and friends

		//ShadowMaps,

		//Render

		//Postprocess
		//Finish,

		//Console,
		//GUI

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
	static constexpr uint32_t ArgumentMemoryBuffer = 1 * 32 * 1024; 
	static constexpr uint32_t CommandLimit = 4096;
	static constexpr uint32_t BytesPerCommand = ArgumentMemoryBuffer / CommandLimit;
};

struct IndexBuffer {
	static constexpr uint32_t TextureBuffer = 256;
};
struct Resources {
	template<size_t SIZE>
	using BitmapAllocator = ::Space::Memory::LinearAtomicBitmapAllocator<SIZE, uint32_t, uint32_t>;

	static constexpr uint32_t TextureLimit = 1024;
	static constexpr uint32_t TextureInitial = 64;
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

struct TextureRenderTask {
	static constexpr uint32_t Limit = 64;
};

} //namespace MoonGlare::Renderer::Configuration
